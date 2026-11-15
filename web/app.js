const $ = (selector) => document.querySelector(selector);
const params = new URLSearchParams(location.search);
const state = {
  projects: [],
  project: Number(params.get('project')) || null,
  event: params.get('event') || '',
  query: params.get('q') || '',
  page: Math.max(0, Number(params.get('page')) || 0),
  request: 0,
};

const labels = {
  IS_CREATED: ['Создан', 'create', '+'],
  IS_DELETED: ['Удалён', 'delete', '−'],
  IS_REPLACED: ['Заменён', 'edit', '⇄'],
  NEW_NAME: ['Переименован', 'move', '↗'],
  NEW_PARENT: ['Перемещён', 'move', '⇄'],
  NEW_HASH: ['Изменено содержимое', 'edit', '✎'],
  NEW_TIME: ['Изменено время', 'edit', '◷'],
};
let lastTimelineKey = '';
let refreshTimer = null;
let refreshInFlight = false;

function node(tag, className, text) {
  const item = document.createElement(tag);
  if (className) item.className = className;
  if (text !== undefined) item.textContent = String(text);
  return item;
}

function notice(message) {
  const element = $('#notice');
  element.hidden = !message;
  element.textContent = message || '';
}

function syncUrl() {
  const next = new URLSearchParams();
  if (state.project) next.set('project', state.project);
  if (state.event) next.set('event', state.event);
  if (state.query) next.set('q', state.query);
  if (state.page) next.set('page', state.page);
  history.replaceState(null, '', `${location.pathname}${next.size ? `?${next}` : ''}`);
}

async function api(url) {
  const response = await fetch(url, { cache: 'no-store' });
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || `Ошибка ${response.status}`);
  return data;
}

function drawProjects() {
  const list = $('#projects');
  list.replaceChildren();
  $('#project-count').textContent = state.projects.length;
  for (const project of state.projects) {
    const button = node('button', `project${project.id === state.project ? ' selected' : ''}`);
    button.type = 'button';
    button.setAttribute('aria-current', project.id === state.project ? 'page' : 'false');
    button.append(node('span', 'project-icon', '⌁'));
    const text = node('span');
    const name = project.path.split('/').filter(Boolean).at(-1) || project.path;
    text.append(node('span', 'project-name', name));
    text.append(node('span', 'project-meta', `${Number(project.changes).toLocaleString('ru-RU')} событий`));
    button.append(text);
    button.title = project.path;
    button.addEventListener('click', () => {
      state.project = project.id;
      state.page = 0;
      drawProjects();
      loadHistory();
    });
    list.append(button);
  }
}

function drawMetrics(project) {
  $('#title').textContent = project ? project.path.split('/').filter(Boolean).at(-1) || project.path : 'Проектов пока нет';
  $('#title').title = project?.path || '';
  $('#subtitle').textContent = project ? project.path : 'Запустите демон, чтобы начать отслеживание.';
  $('#subtitle').title = project?.path || '';
  $('#metric-files').textContent = project ? Number(project.files).toLocaleString('ru-RU') : '0';
  $('#metric-changes').textContent = project ? Number(project.changes).toLocaleString('ru-RU') : '0';
  $('#metric-last').textContent = project?.last_change || '—';
}

function detailText(change) {
  let detail;
  try { detail = JSON.parse(change.details || '{}'); } catch { return change.details || ''; }
  if (change.event === 'NEW_NAME' && detail.name) return `Прежнее имя: ${detail.name}`;
  if (change.event === 'NEW_PARENT' && detail.parent) return `Прежний ID каталога: ${detail.parent}`;
  if (change.event === 'NEW_HASH' && detail.crc) return `Предыдущий хеш: ${detail.crc}`;
  if (change.event === 'IS_REPLACED' && detail.old_uuid) return `Заменил файл UUID ${detail.old_uuid}`;
  return '';
}

function dayLabel(date) {
  const parsed = new Date(`${date}T12:00:00`);
  return Number.isNaN(parsed.valueOf()) ? date : new Intl.DateTimeFormat('ru-RU', {
    day: 'numeric', month: 'long', year: 'numeric',
  }).format(parsed);
}

function drawTimeline(data) {
  const list = $('#timeline');
  const key = JSON.stringify([state.project, state.event, state.query, state.page, data.total, data.changes]);
  if (key === lastTimelineKey) return;
  const expandedIds = new Set([...list.querySelectorAll('.entry[open]')].map((item) => item.dataset.id));
  lastTimelineKey = key;
  list.replaceChildren();
  $('#filtered-count').textContent = `${Number(data.total).toLocaleString('ru-RU')} записей`;
  const pages = Math.max(1, Math.ceil(data.total / data.page_size));
  $('#page-info').textContent = `${state.page + 1} / ${pages}`;
  $('#prev').disabled = state.page === 0;
  $('#next').disabled = state.page + 1 >= pages;
  if (!data.changes.length) {
    const empty = node('div', 'empty');
    empty.append(node('strong', '', 'События не найдены'));
    empty.append('Попробуйте другой фильтр или поисковый запрос.');
    list.append(empty);
    return;
  }

  let currentDay = null;
  for (const change of data.changes) {
    const date = (change.occurred || '').slice(0, 10);
    if (date !== currentDay) {
      currentDay = date;
      list.append(node('div', 'day', dayLabel(date)));
    }
    const [label, kind, symbol] = labels[change.event] || [change.event, 'edit', '·'];
    const entry = node('details', `entry ${kind}`);
    entry.dataset.id = String(change.id);
    entry.open = expandedIds.has(entry.dataset.id);
    const summary = node('summary', 'entry-summary');
    summary.append(node('span', 'entry-symbol', symbol));
    const overview = node('span', 'entry-overview');
    const main = node('span', 'entry-main');
    main.append(node('span', 'event-label', label));
    const filename = node('span', 'entry-name', change.name || change.current_path || 'Без имени');
    filename.title = change.name || change.current_path || '';
    main.append(filename);
    overview.append(main);
    const parentPath = (change.current_path || '').split('/').slice(0, -1).join('/');
    const segments = parentPath.split('/').filter(Boolean);
    const shortPath = segments.length > 3 ? `…/${segments.slice(-3).join('/')}` : parentPath;
    const path = node('span', 'entry-location', shortPath || 'Корень проекта');
    path.title = change.current_path || '';
    overview.append(path);
    summary.append(overview);
    const time = node('time', 'entry-time', (change.occurred || '').slice(11, 19));
    time.dateTime = (change.occurred || '').replace(' ', 'T');
    summary.append(time);
    summary.append(node('span', 'entry-chevron', '⌄'));
    entry.append(summary);

    const details = node('div', 'entry-details');
    details.append(node('div', 'entry-path', change.current_path || ''));
    const meta = node('div', 'entry-sub');
    const detail = detailText(change);
    if (detail) meta.append(node('span', 'detail', detail));
    if (change.uuid) {
      meta.append(node('code', '', `UUID ${change.uuid}`));
    }
    if (change.deleted && change.event !== 'IS_DELETED') meta.append(node('span', '', 'Позже удалён'));
    details.append(meta);
    entry.append(details);
    list.append(entry);
  }
}

async function loadHistory() {
  const request = ++state.request;
  const project = state.projects.find((item) => item.id === state.project);
  drawMetrics(project);
  syncUrl();
  if (!project) {
    lastTimelineKey = '';
    $('#timeline').replaceChildren(node('div', 'empty', 'Пока нет отслеживаемых проектов.'));
    $('#filtered-count').textContent = '';
    $('#page-info').textContent = '—';
    $('#prev').disabled = $('#next').disabled = true;
    return;
  }
  notice('');
  const query = new URLSearchParams({ project: state.project, page: state.page, event: state.event, q: state.query });
  try {
    const data = await api(`/api/history?${query}`);
    if (request === state.request) drawTimeline(data);
  } catch (error) {
    if (request === state.request) notice(error.message);
  }
}

async function loadProjects() {
  notice('');
  try {
    state.projects = await api('/api/projects');
    if (!state.projects.some((project) => project.id === state.project)) {
      state.project = state.projects[0]?.id || null;
      state.page = 0;
    }
    drawProjects();
    await loadHistory();
  } catch (error) {
    notice(error.message);
  }
}

function scheduleRefresh() {
  clearTimeout(refreshTimer);
  refreshTimer = null;
  if ($('#auto-refresh').checked) {
    const period = Number($('#refresh-period').value);
    refreshTimer = setTimeout(refreshData, period * 1000);
  }
}

async function refreshData() {
  if (refreshInFlight) return;
  clearTimeout(refreshTimer);
  refreshTimer = null;
  refreshInFlight = true;
  try {
    await loadProjects();
  } finally {
    refreshInFlight = false;
    scheduleRefresh();
  }
}

$('#filters').addEventListener('click', (event) => {
  const button = event.target.closest('button[data-event]');
  if (!button) return;
  state.event = button.dataset.event;
  state.page = 0;
  document.querySelectorAll('.filter').forEach((item) => item.classList.toggle('active', item === button));
  loadHistory();
});
if (!labels[state.event]) state.event = '';
document.querySelectorAll('.filter').forEach((item) => item.classList.toggle('active', item.dataset.event === state.event));
$('#search').value = state.query;
let debounce;
$('#search').addEventListener('input', (event) => {
  clearTimeout(debounce);
  debounce = setTimeout(() => {
    state.query = event.target.value.trim();
    state.page = 0;
    loadHistory();
  }, 250);
});
$('#prev').addEventListener('click', () => { state.page--; loadHistory(); });
$('#next').addEventListener('click', () => { state.page++; loadHistory(); });
$('#refresh').addEventListener('click', refreshData);
$('#auto-refresh').addEventListener('change', scheduleRefresh);
$('#refresh-period').addEventListener('change', scheduleRefresh);
refreshData();
