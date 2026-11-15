"""Read-only history viewer for the undefinedSpace MariaDB database."""

import json
import logging
import os
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlsplit

import MySQLdb
import MySQLdb.cursors


HERE = Path(__file__).resolve().parent
PAGE_SIZE = 30
EVENTS = {"IS_CREATED", "IS_DELETED", "IS_REPLACED", "NEW_NAME", "NEW_HASH", "NEW_PARENT", "NEW_TIME"}


def connection():
    return MySQLdb.connect(
        host=os.getenv("UNDEFINED_DB_HOST", "127.0.0.1"),
        port=int(os.getenv("UNDEFINED_DB_PORT", "3306")),
        user=os.environ["UNDEFINED_DB_USER"],
        passwd=os.environ["UNDEFINED_DB_PASSWORD"],
        db=os.getenv("UNDEFINED_DB_NAME", "undefinedSpace"),
        charset="utf8mb4",
        cursorclass=MySQLdb.cursors.DictCursor,
    )


def projects(db):
    with db.cursor() as cursor:
        cursor.execute(
            """SELECT p.id, p.path,
                      DATE_FORMAT(p.time_start, '%Y-%m-%d %H:%i:%s') AS started,
                      COUNT(DISTINCT active_item.id) AS files,
                      COUNT(DISTINCT c.id) AS changes,
                      DATE_FORMAT(MAX(c.time), '%Y-%m-%d %H:%i:%s') AS last_change
               FROM projects p
               LEFT JOIN accords a ON a.id_project = p.id
               LEFT JOIN items active_item ON active_item.id = a.id_item AND active_item.deleted = 0
               LEFT JOIN changes c ON c.id_item = a.id_item
               GROUP BY p.id, p.path, p.time_start
               ORDER BY last_change DESC, p.id DESC"""
        )
        return cursor.fetchall()


def history(db, project_id, event, query, page):
    condition = "a.id_project = %s"
    params = [project_id]
    if event:
        condition += " AND e.description = %s"
        params.append(event)
    if query:
        condition += " AND (i.name LIKE %s OR i.uuid LIKE %s OR c.description LIKE %s OR e.description LIKE %s)"
        pattern = f"%{query}%"
        params.extend((pattern, pattern, pattern, pattern))

    with db.cursor() as cursor:
        cursor.execute("SELECT path FROM projects WHERE id = %s", (project_id,))
        project = cursor.fetchone()
        if project is None:
            return None

        cursor.execute(
            """SELECT COUNT(*) AS total
               FROM changes c
               JOIN accords a ON a.id_item = c.id_item
               JOIN items i ON i.id = c.id_item
               LEFT JOIN events e ON e.id = c.id_event
               WHERE """ + condition,
            tuple(params),
        )
        total = cursor.fetchone()["total"]

        # Paths are reconstructed from current parent links. Historic paths
        # cannot be recovered reliably from the existing changes table.
        cursor.execute(
            """WITH RECURSIVE paths AS (
                 SELECT i.id, CAST(p.path AS CHAR(4096)) AS path, 0 AS depth
                 FROM projects p JOIN items i ON i.id = p.id_item
                 WHERE p.id = %s
                 UNION ALL
                 SELECT child.id, CONCAT(paths.path, '/', child.name), paths.depth + 1
                 FROM items child JOIN paths ON child.id_parent = paths.id
                 WHERE paths.depth < 64
               )
               SELECT c.id, DATE_FORMAT(c.time, '%%Y-%%m-%%d %%H:%%i:%%s') AS occurred,
                      COALESCE(e.description, 'UNKNOWN') AS event,
                      c.description AS details, i.uuid, i.name, i.deleted,
                      COALESCE(paths.path, i.name) AS `current_path`
               FROM changes c
               JOIN accords a ON a.id_item = c.id_item
               JOIN items i ON i.id = c.id_item
               LEFT JOIN events e ON e.id = c.id_event
               LEFT JOIN paths ON paths.id = i.id
               WHERE """ + condition + " ORDER BY c.time DESC, c.id DESC LIMIT %s OFFSET %s",
            (project_id, *params, PAGE_SIZE, page * PAGE_SIZE),
        )
        return {"project": project, "total": total, "page": page,
                "page_size": PAGE_SIZE, "changes": cursor.fetchall()}


class Handler(BaseHTTPRequestHandler):
    def respond(self, status, body, content_type):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; "
                         "style-src 'self'; connect-src 'self'; img-src 'self' data:")
        self.end_headers()
        self.wfile.write(body)

    def send_json(self, status, data):
        self.respond(status, json.dumps(data, ensure_ascii=False).encode("utf-8"),
                     "application/json; charset=utf-8")

    def do_GET(self):
        url = urlsplit(self.path)
        static = {"/": ("index.html", "text/html"),
                  "/app.js": ("app.js", "text/javascript"),
                  "/style.css": ("style.css", "text/css")}
        if url.path in static:
            filename, mime = static[url.path]
            self.respond(200, (HERE / filename).read_bytes(), mime + "; charset=utf-8")
            return
        if url.path not in ("/api/projects", "/api/history"):
            self.send_json(404, {"error": "Не найдено"})
            return
        try:
            if url.path == "/api/history":
                args = parse_qs(url.query)
                project_id = int(args.get("project", [""])[0])
                page = int(args.get("page", ["0"])[0])
                event = args.get("event", [""])[0]
                query = args.get("q", [""])[0].strip()[:100]
                if project_id <= 0 or not 0 <= page <= 100000 or event and event not in EVENTS:
                    raise ValueError("Invalid query")
            with connection() as db:
                data = projects(db) if url.path == "/api/projects" else history(
                    db, project_id, event, query, page)
            if data is None:
                self.send_json(404, {"error": "Проект не найден"})
            else:
                self.send_json(200, data)
        except (ValueError, IndexError):
            self.send_json(400, {"error": "Неверные параметры запроса"})
        except (MySQLdb.Error, KeyError):
            logging.exception("Database query failed")
            self.send_json(503, {"error": "База недоступна; проверьте настройки сервера"})


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    host = os.getenv("UNDEFINED_WEB_HOST", "127.0.0.1")
    port = int(os.getenv("UNDEFINED_WEB_PORT", "8080"))
    print(f"History viewer: http://{host}:{port}/", flush=True)
    ThreadingHTTPServer((host, port), Handler).serve_forever()
