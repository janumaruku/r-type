'use strict';

const METRICS = [
    {key: 'lines', label: 'Lines', series: '--series-1'},
    {key: 'branches', label: 'Branches', series: '--series-2'},
    {key: 'functions', label: 'Functions', series: '--series-3'},
    {key: 'regions', label: 'Regions', series: '--series-4'},
];
const TABLE_METRICS = ['lines', 'functions', 'regions', 'branches'];
const TOP_UNCOVERED = 10;

const state = {data: null, history: [], charts: [], query: '', module: '', sortKey: 'lines', sortDirection: 1};

function percent(metric) {
    return metric.count === 0 ? null : (100 * metric.covered) / metric.count;
}

function formatPercent(value) {
    return value === null ? '—' : `${value.toFixed(1)}%`;
}

function level(value) {
    if (value === null) {
        return 'none';
    }
    if (value >= 80) {
        return 'good';
    }
    return value >= 60 ? 'fair' : 'poor';
}

function cssVar(name) {
    return getComputedStyle(document.documentElement).getPropertyValue(name).trim();
}

function escapeHtml(text) {
    return String(text).replace(/[&<>"']/g, (character) => `&#${character.charCodeAt(0)};`);
}

function shortDate(iso) {
    return iso.slice(0, 10);
}

function track(chart) {
    state.charts.push(chart);
    return chart;
}

function applyChartDefaults() {
    Chart.defaults.color = cssVar('--muted');
    Chart.defaults.borderColor = cssVar('--grid');
    Chart.defaults.font.family = getComputedStyle(document.body).fontFamily;
    Chart.defaults.maintainAspectRatio = false;
}

function percentAxis() {
    return {min: 0, max: 100, ticks: {callback: (value) => `${value}%`}};
}

function renderSource() {
    const {data} = state;
    const commit = data.commit.slice(0, 7);
    const commitLink = data.repositoryUrl
        ? `<a href="${escapeHtml(data.repositoryUrl)}/commit/${escapeHtml(data.commit)}"><code>${commit}</code></a>`
        : `<code>${commit}</code>`;
    const origin = data.event === 'pull_request' && data.pullRequest
        ? `pull request <a href="${escapeHtml(data.repositoryUrl)}/pull/${escapeHtml(data.pullRequest)}">#${escapeHtml(data.pullRequest)}</a> (<code>${escapeHtml(data.branch)}</code>)`
        : `<code>${escapeHtml(data.branch)}</code>`;
    const date = new Date(data.generated).toLocaleString();

    document.getElementById('source').innerHTML = `From ${origin} at ${commitLink} · ${escapeHtml(date)}`;
    if (data.runUrl) {
        const runLink = document.getElementById('run-link');

        runLink.href = data.runUrl;
        runLink.hidden = false;
    }
}

function renderRings() {
    const container = document.getElementById('rings');

    container.innerHTML = '';
    for (const metric of METRICS) {
        const total = state.data.totals[metric.key];
        const value = percent(total);
        const card = document.createElement('div');

        card.className = 'card ring-card';
        card.innerHTML = `
            <div class="ring"><canvas></canvas><span class="ring-value">${formatPercent(value)}</span></div>
            <div>
                <div class="ring-label">${metric.label}</div>
                <div class="ring-detail">${total.covered.toLocaleString()} of ${total.count.toLocaleString()} covered</div>
            </div>`;
        container.append(card);
        track(new Chart(card.querySelector('canvas'), {
            type: 'doughnut',
            data: {
                datasets: [{
                    data: [total.covered, total.count ? total.count - total.covered : 1],
                    backgroundColor: [cssVar(`--${level(value)}`), cssVar('--track')],
                    borderWidth: 0,
                }],
            },
            options: {cutout: '78%', events: [], plugins: {legend: {display: false}, tooltip: {enabled: false}}},
        }));
    }
}

function renderHistory() {
    const history = state.history;
    const box = document.getElementById('history-box');
    const empty = document.getElementById('history-empty');

    if (history.length < 2) {
        box.hidden = true;
        empty.hidden = false;
        return;
    }
    box.hidden = false;
    empty.hidden = true;
    track(new Chart(document.getElementById('history-chart'), {
        type: 'line',
        data: {
            labels: history.map((entry) => shortDate(entry.date)),
            datasets: METRICS.map((metric) => ({
                label: metric.label,
                data: history.map((entry) => percent(entry.totals[metric.key])),
                borderColor: cssVar(metric.series),
                backgroundColor: cssVar(metric.series),
                tension: 0.25,
                pointRadius: history.length > 60 ? 0 : 3,
                spanGaps: true,
            })),
        },
        options: {
            interaction: {mode: 'index', intersect: false},
            scales: {y: percentAxis()},
            plugins: {
                tooltip: {
                    callbacks: {
                        title: (items) => {
                            const entry = history[items[0].dataIndex];

                            return `${shortDate(entry.date)} · ${entry.commit.slice(0, 7)}`;
                        },
                        label: (item) => ` ${item.dataset.label}: ${formatPercent(item.parsed.y)}`,
                    },
                },
            },
        },
    }));
}

function renderModules() {
    const modules = [...state.data.modules].sort((a, b) => (percent(b.lines) ?? -1) - (percent(a.lines) ?? -1));
    const shown = METRICS.filter((metric) => metric.key === 'lines' || metric.key === 'branches');

    document.getElementById('modules-box').style.height = `${Math.max(220, modules.length * 44 + 70)}px`;
    track(new Chart(document.getElementById('modules-chart'), {
        type: 'bar',
        data: {
            labels: modules.map((module) => module.name),
            datasets: shown.map((metric) => ({
                label: metric.label,
                data: modules.map((module) => percent(module[metric.key])),
                backgroundColor: cssVar(metric.series),
                borderRadius: 4,
            })),
        },
        options: {
            indexAxis: 'y',
            scales: {x: percentAxis()},
            plugins: {
                tooltip: {
                    callbacks: {
                        label: (item) => {
                            const metric = modules[item.dataIndex][shown[item.datasetIndex].key];

                            return ` ${item.dataset.label}: ${formatPercent(item.parsed.x)} (${metric.covered}/${metric.count})`;
                        },
                    },
                },
            },
        },
    }));
}

function renderUncovered() {
    const files = state.data.files
        .map((file) => ({...file, uncovered: file.lines.count - file.lines.covered}))
        .filter((file) => file.uncovered > 0)
        .sort((a, b) => b.uncovered - a.uncovered)
        .slice(0, TOP_UNCOVERED);

    document.getElementById('uncovered-box').style.height = `${Math.max(220, files.length * 34 + 60)}px`;
    track(new Chart(document.getElementById('uncovered-chart'), {
        type: 'bar',
        data: {
            labels: files.map((file) => file.path.split('/').pop()),
            datasets: [{
                label: 'Uncovered lines',
                data: files.map((file) => file.uncovered),
                backgroundColor: files.map((file) => cssVar(`--${level(percent(file.lines))}`)),
                borderRadius: 4,
            }],
        },
        options: {
            indexAxis: 'y',
            plugins: {
                legend: {display: false},
                tooltip: {
                    callbacks: {
                        title: (items) => files[items[0].dataIndex].path,
                        label: (item) => {
                            const file = files[item.dataIndex];

                            return ` ${file.uncovered} of ${file.lines.count} lines uncovered (${formatPercent(percent(file.lines))} covered)`;
                        },
                    },
                },
            },
            onClick: (_event, elements) => {
                if (elements.length > 0) {
                    window.location.href = files[elements[0].index].link;
                }
            },
            onHover: (event, elements) => {
                event.native.target.style.cursor = elements.length > 0 ? 'pointer' : 'default';
            },
        },
    }));
}

function metricCell(metric) {
    const value = percent(metric);
    const width = value === null ? 0 : value;

    return `<td><div class="metric">
        <span class="bar"><span class="level-${level(value)}" style="width:${width}%"></span></span>
        <span class="value">${formatPercent(value)}</span>
        <span class="fraction">${metric.covered}/${metric.count}</span>
    </div></td>`;
}

function sortValue(file, key) {
    return key === 'path' ? file.path : (percent(file[key]) ?? -1);
}

function renderTable() {
    const query = state.query.toLowerCase();
    const files = state.data.files
        .filter((file) => (!state.module || file.module === state.module) && file.path.toLowerCase().includes(query))
        .sort((a, b) => {
            const left = sortValue(a, state.sortKey);
            const right = sortValue(b, state.sortKey);

            if (left === right) {
                return a.path.localeCompare(b.path);
            }
            return (left < right ? -1 : 1) * state.sortDirection;
        });

    document.querySelector('#files tbody').innerHTML = files.map((file) => `<tr>
        <td class="path"><a href="${escapeHtml(file.link)}">${escapeHtml(file.path)}</a><span class="module">${escapeHtml(file.module)}</span></td>
        ${TABLE_METRICS.map((key) => metricCell(file[key])).join('')}
    </tr>`).join('');
    document.getElementById('files-count').textContent = `${files.length} of ${state.data.files.length} files`;
    for (const header of document.querySelectorAll('#files th[data-sort]')) {
        header.removeAttribute('aria-sort');
        if (header.dataset.sort === state.sortKey) {
            header.setAttribute('aria-sort', state.sortDirection > 0 ? 'ascending' : 'descending');
        }
    }
}

function setUpTable() {
    const select = document.getElementById('module-filter');

    for (const module of state.data.modules) {
        select.append(new Option(module.name, module.name));
    }
    select.addEventListener('change', () => {
        state.module = select.value;
        renderTable();
    });
    document.getElementById('search').addEventListener('input', (event) => {
        state.query = event.target.value;
        renderTable();
    });
    for (const header of document.querySelectorAll('#files th[data-sort]')) {
        header.addEventListener('click', () => {
            const key = header.dataset.sort;

            state.sortDirection = state.sortKey === key ? -state.sortDirection : 1;
            state.sortKey = key;
            renderTable();
        });
    }
    renderTable();
}

function renderCharts() {
    for (const chart of state.charts) {
        chart.destroy();
    }
    state.charts = [];
    applyChartDefaults();
    renderRings();
    renderHistory();
    renderModules();
    renderUncovered();
}

async function fetchJson(path, fallback) {
    const response = await fetch(path, {cache: 'no-cache'});

    if (!response.ok) {
        if (fallback !== undefined) {
            return fallback;
        }
        throw new Error(`${path}: HTTP ${response.status}`);
    }
    return response.json();
}

async function main() {
    try {
        [state.data, state.history] = await Promise.all([fetchJson('data.json'), fetchJson('history.json', [])]);
    } catch (error) {
        const message = document.getElementById('error');

        message.textContent = `Could not load the coverage data (${error.message}).`;
        message.hidden = false;
        document.getElementById('source').textContent = 'No report';
        return;
    }
    renderSource();
    renderCharts();
    setUpTable();
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', renderCharts);
}

main();
