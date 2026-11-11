import hashlib, json, os, sys
DB = 'build_ios/file_mtimes.json'
mode = sys.argv[1]
old = json.load(open(DB)) if os.path.exists(DB) else {}
cur = {}
for root, dirs, files in os.walk('..'):
    if 'build_ios' in root or '.git' in root: continue
    for fn in files:
        p = os.path.join(root, fn)
        try:
            h = hashlib.md5(open(p, 'rb').read()).hexdigest()
            mt = os.path.getmtime(p)
            if mode == 'restore' and p in old and old[p][0] == h:
                mt = old[p][1]
                os.utime(p, (mt, mt))
            cur[p] = [h, mt]
        except Exception: pass
if mode == 'save':
    os.makedirs('build_ios', exist_ok=True)
    json.dump(cur, open(DB, 'w'))
