import json
import argparse
import sys
from collections import defaultdict

# -----------------------------------------------------------------------------
# HTML TEMPLATE
# -----------------------------------------------------------------------------
HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>HPerf Report</title>
    <script src="https://cdn.jsdelivr.net/npm/echarts@5.4.3/dist/echarts.min.js"></script>
    <style>
        :root { --primary: #2563eb; --bg: #f8fafc; --panel: #ffffff; --border: #cbd5e1; --text: #334155; }
        body { margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: var(--bg); color: var(--text); height: 100vh; display: flex; flex-direction: column; overflow: hidden; }
        
        /* HEADER */
        header { background: var(--panel); border-bottom: 1px solid var(--border); padding: 0 20px; height: 50px; display: flex; align-items: center; justify-content: space-between; z-index: 20; box-shadow: 0 1px 2px rgba(0,0,0,0.05); }
        .controls { display: flex; gap: 10px; }
        button { padding: 6px 16px; border: 1px solid #cbd5e1; background: #fff; cursor: pointer; border-radius: 6px; font-weight: 500; font-size: 13px; color: #475569; transition: all 0.1s; }
        button:hover { background: #f1f5f9; border-color: #94a3b8; }
        button.active { background: var(--primary); color: white; border-color: var(--primary); box-shadow: 0 1px 2px rgba(37,99,235,0.2); }
        .stats { font-size: 12px; font-family: 'Monaco', monospace; display: flex; gap: 20px; color: #64748b; }
        .stats b { color: var(--text); }

        /* VIEW CONTAINER */
        #container { position: relative; flex: 1; width: 100%; height: 100%; overflow: hidden; }
        .view { position: absolute; top: 0; left: 0; width: 100%; height: 100%; background: var(--bg); display: none; }
        .view.active { display: block; }
        .chart-box { width: 100%; height: 100%; }

        /* --- FIXED TABLE STYLING --- */
        #table-view { overflow: auto; padding: 0; background: #fff; }
        
        table { 
            width: 100%; 
            border-collapse: collapse; 
            table-layout: fixed; /* Crucial for alignment */
            min-width: 800px; /* Prevent crushing on small screens */
        }
        
        thead th { 
            position: sticky; 
            top: 0; 
            background: #f1f5f9; 
            z-index: 10; 
            padding: 12px 15px;
            text-align: left;
            font-size: 12px;
            text-transform: uppercase;
            font-weight: 600;
            color: #64748b;
            border-bottom: 1px solid #cbd5e1;
            box-shadow: 0 1px 2px rgba(0,0,0,0.05);
            cursor: pointer;
            user-select: none;
        }
        thead th:hover { background: #e2e8f0; color: #334155; }

        tbody tr { border-bottom: 1px solid #f1f5f9; transition: background 0.1s; }
        tbody tr:hover { background-color: #f8fafc; }
        
        tbody td { 
            padding: 8px 15px; 
            font-size: 13px; 
            color: #334155;
            white-space: nowrap; 
            overflow: hidden; 
            text-overflow: ellipsis; /* Truncate long names */
        }

        /* Column Widths */
        th:nth-child(1), td:nth-child(1) { width: 55%; } /* Function Name */
        th:nth-child(2), td:nth-child(2) { width: 15%; text-align: right; } /* Count */
        th:nth-child(3), td:nth-child(3) { width: 15%; text-align: right; } /* Total */
        th:nth-child(4), td:nth-child(4) { width: 15%; text-align: right; } /* Self */

        .num { font-family: 'Monaco', monospace; }
        .func-name { font-family: 'Monaco', monospace; font-size: 12px; color: var(--primary); }
        .dot { display: inline-block; width: 8px; height: 8px; border-radius: 2px; margin-right: 8px; }
    </style>
</head>
<body>

<header>
    <div class="controls">
        <button class="tab-btn active" onclick="setTab('timeline')">Memory Timeline</button>
        <button class="tab-btn" onclick="setTab('flame')">Aggregated Flame Graph</button>
        <button class="tab-btn" onclick="setTab('table')">Function Stats</button>
    </div>
    <div class="stats">
        <span>Time: <b id="meta-time">0</b> ms</span>
        <span>Max Mem: <b id="meta-mem">0</b> B</span>
        <span>Events: <b id="meta-evt">0</b></span>
    </div>
</header>

<div id="container">
    <!-- TIMELINE -->
    <div id="timeline-view" class="view active"><div id="timeline-chart" class="chart-box"></div></div>
    
    <!-- FLAME -->
    <div id="flame-view" class="view"><div id="flame-chart" class="chart-box"></div></div>
    
    <!-- TABLE -->
    <div id="table-view" class="view">
        <table id="stats-table">
            <thead>
                <tr>
                    <th onclick="sortT(0)">Function Name</th>
                    <th class="num" onclick="sortT(1)">Count</th>
                    <th class="num" onclick="sortT(2)">Total Time (ms)</th>
                    <th class="num" onclick="sortT(3)">Self Time (ms)</th>
                </tr>
            </thead>
            <tbody id="tbody"></tbody>
        </table>
    </div>
</div>

<script>
    const RAW_MEM = REPLACE_MEMORY; 
    const RAW_FL = REPLACE_FLAME;    
    const NAMES = REPLACE_NAMES;
    const STATS = REPLACE_STATS;
    const META = REPLACE_META;

    document.getElementById('meta-time').innerText = (META.duration_ns/1e6).toFixed(2);
    document.getElementById('meta-mem').innerText = META.max_memory.toLocaleString();
    document.getElementById('meta-evt').innerText = META.num_events.toLocaleString();

    const colorCache = NAMES.map(n => {
        let h = 0; for(let i=0;i<n.length;i++) h = n.charCodeAt(i) + ((h<<5)-h);
        return `hsl(${h % 360}, 75%, 60%)`;
    });

    // --- 1. Memory Chart ---
    let tlChart = null;
    function initTL() {
        if(tlChart) return;
        tlChart = echarts.init(document.getElementById('timeline-chart'));
        tlChart.setOption({
            tooltip: { trigger: 'axis', formatter: p => `Time: ${p[0].axisValue} ms<br><b>${p[0].value[1].toLocaleString()} B</b>` },
            grid: { top: 40, bottom: 60, left: 60, right: 20 },
            dataZoom: [{ type: 'inside' }, { type: 'slider', bottom: 10 }],
            xAxis: { type: 'value', scale: true },
            yAxis: { type: 'value', splitLine: { lineStyle: { type: 'dashed' } } },
            series: [{
                type: 'line', showSymbol: false, step: 'end', data: RAW_MEM,
                lineStyle: { width: 2, color: '#ef4444' },
                areaStyle: { color: new echarts.graphic.LinearGradient(0,0,0,1, [{offset:0, color:'rgba(239,68,68,0.5)'}, {offset:1, color:'rgba(239,68,68,0.05)'}]) }
            }]
        });
        window.onresize = () => tlChart && tlChart.resize();
    }

    // --- 2. Flame Chart ---
    let flChart = null;
    function initFL() {
        if(flChart) return;
        flChart = echarts.init(document.getElementById('flame-chart'));
        flChart.setOption({
            tooltip: { formatter: p => `<b>${NAMES[p.value[3]]}</b><br>Total: ${p.value[1].toFixed(3)} ms` },
            grid: { top: 20, bottom: 40, left: 10, right: 10 },
            dataZoom: [{ type: 'inside' }, { type: 'slider', bottom: 10 }],
            xAxis: { type: 'value', min: 0, scale: true, show: false },
            yAxis: { type: 'value', inverse: true, show: false },
            series: [{
                type: 'custom',
                renderItem: (params, api) => {
                    const start = api.value(0), dur = api.value(1), depth = api.value(2), nid = api.value(3);
                    const tl = api.coord([start, depth]), br = api.coord([start + dur, depth+1]);
                    const w = br[0] - tl[0];
                    if (w < 2) return;
                    return {
                        type: 'rect', shape: { x: tl[0], y: tl[1], width: w, height: br[1] - tl[1] - 1 },
                        style: { fill: colorCache[nid] },
                        textContent: { style: { text: w > 50 ? NAMES[nid] : '', fill: '#fff', truncate: { maxWidth: w - 5 } } },
                        textConfig: { position: 'insideLeft', distance: 4 }
                    };
                },
                data: RAW_FL, progressive: 5000
            }]
        });
        window.onresize = () => flChart && flChart.resize();
    }

    // --- 3. Stats Table ---
    function initTab() {
        STATS.sort((a,b) => b.total - a.total);
        renderTab(STATS);
    }
    
    function renderTab(data) {
        // Optimized HTML generation
        const html = data.map(i => {
            const color = colorCache[NAMES.indexOf(i.name)];
            // Use title attribute for hover tooltip on long names
            return `<tr>
                <td title="${i.name}">
                    <span class="dot" style="background:${color}"></span>
                    <span class="func-name">${i.name}</span>
                </td>
                <td class="num">${i.count.toLocaleString()}</td>
                <td class="num">${i.total.toFixed(3)}</td>
                <td class="num" style="color:#64748b">${i.self.toFixed(3)}</td>
            </tr>`;
        }).join('');
        document.getElementById('tbody').innerHTML = html;
    }

    let sDir = 1;
    window.sortT = function(idx) {
        sDir *= -1;
        STATS.sort((a,b) => {
            const v = (x) => idx===0?x.name:idx===1?x.count:idx===2?x.total:x.self;
            return idx===0 ? v(a).localeCompare(v(b))*sDir : (v(a)-v(b))*sDir;
        });
        renderTab(STATS);
    }

    // --- Switcher ---
    window.setTab = function(id) {
        document.querySelectorAll('.view').forEach(e => e.classList.remove('active'));
        document.querySelectorAll('.tab-btn').forEach(e => e.classList.remove('active'));
        document.getElementById(id + '-view').classList.add('active');
        event.target.classList.add('active');
        
        if(id === 'timeline') initTL();
        if(id === 'flame') {
            document.getElementById('flame-view').style.display = 'block'; // Force layout before init
            initFL();
            if(flChart) flChart.resize(); // Fix canvas size if hidden previously
        }
        if(id === 'table') initTab();
    };

    initTL();
</script>
</body>
</html>
"""

def process():
    parser = argparse.ArgumentParser(sys.argv[0], description='HTML report generator for collected hperf data.')
    parser.add_argument('input', help='input JSON file')
    parser.add_argument('-o', default='hperf_report.html', help='output file (default: hperf_report.html)')
    args = parser.parse_args()

    with open(args.input) as f: data = json.load(f)
    
    events = sorted(data.get('events', []), key=lambda x: (x['timestamp']['sec'], x['timestamp']['nsec']))
    if not events: return
    
    start_ns = (events[0]['timestamp']['sec']*1e9) + events[0]['timestamp']['nsec']
    
    memory, flame_tree = [], defaultdict(int)
    stats = defaultdict(lambda: {'count':0, 'total':0, 'self':0})
    names, name_map = [], {}

    def get_id(n):
        if n not in name_map:
            name_map[n] = len(names)
            names.append(n)
        return name_map[n]

    stack = []
    cur_mem, max_mem = 0, 0
    allocs = {}
    memory.append([0, 0])

    for e in events:
        ts = (e['timestamp']['sec']*1e9) + e['timestamp']['nsec']
        rel = ts - start_ns
        typ = e['type']

        if typ == 'func-enter':
            nid = get_id(e.get('func-name', '?'))
            stack.append({'nid': nid, 'start': rel, 'child_ns': 0})
        
        elif typ == 'func-leave':
            if stack:
                frame = stack.pop()
                dur = rel - frame['start']
                
                nm = names[frame['nid']]
                stats[nm]['count'] += 1
                stats[nm]['total'] += dur
                stats[nm]['self'] += max(0, dur - frame['child_ns'])

                path = "/".join([names[f['nid']] for f in stack] + [nm])
                flame_tree[path] += dur

                if stack: stack[-1]['child_ns'] += dur

        elif typ == 'mem-alloc':
            ptr, sz = e.get('ptr'), e.get('size', 0)
            allocs[ptr] = sz
            cur_mem += sz
            max_mem = max(max_mem, cur_mem)
            memory.append([rel/1e6, cur_mem])

        elif typ == 'mem-free':
            ptr = e.get('ptr')
            if ptr in allocs:
                cur_mem = max(0, cur_mem - allocs.pop(ptr))
                memory.append([rel/1e6, cur_mem])

    last_rel = (events[-1]['timestamp']['sec']*1e9 + events[-1]['timestamp']['nsec']) - start_ns
    while stack:
        frame = stack.pop()
        dur = last_rel - frame['start']
        nid = get_id(names[frame['nid']] + " (inc)")
        flame_tree["/".join([names[f['nid']] for f in stack] + [names[nid]])] += dur

    flame_data = []
    root = {'val': 0, 'children': {}}
    for path, val in flame_tree.items():
        parts = path.split("/")
        curr = root
        for p in parts:
            if p not in curr['children']: curr['children'][p] = {'val': 0, 'children': {}, 'name': p}
            curr = curr['children'][p]
        curr['val'] = val

    def traverse(node, offset, depth):
        if depth >= 0:
            flame_data.append([offset, node['val']/1e6, depth, get_id(node['name'])])
        child_offset = offset
        for child in sorted(node['children'].values(), key=lambda x: x['val'], reverse=True):
            traverse(child, child_offset, depth+1)
            child_offset += (child['val']/1e6)

    for c in sorted(root['children'].values(), key=lambda x: x['val'], reverse=True):
        traverse(c, 0, 0)

    stat_list = [{'name':k, 'count':v['count'], 'total':v['total']/1e6, 'self':v['self']/1e6} for k,v in stats.items()]
    meta = {'duration_ns': last_rel, 'max_memory': max_mem, 'num_events': len(events)}

    html = HTML_TEMPLATE.replace('REPLACE_MEMORY', json.dumps(memory)) \
                        .replace('REPLACE_FLAME', json.dumps(flame_data)) \
                        .replace('REPLACE_NAMES', json.dumps(names)) \
                        .replace('REPLACE_STATS', json.dumps(stat_list)) \
                        .replace('REPLACE_META', json.dumps(meta))
    
    with open(args.o, 'w') as f: f.write(html)
    print(f"Generated report: {args.o}")

if __name__ == '__main__': process()