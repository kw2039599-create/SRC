import hashlib, json, os, shutil, sys, time

DB = 'build_ios/file_mtimes.json'
CACHE_EPOCH = 'v2_clean_rebuild_20261111'
mode = sys.argv[1]

old_db = {}
if os.path.exists(DB):
    try:
        old_db = json.load(open(DB))
    except Exception:
        old_db = {}

old_files = old_db.get('files', {}) if isinstance(old_db, dict) and 'files' in old_db else {}
old_epoch = old_db.get('epoch', '') if isinstance(old_db, dict) else ''
old_hdr_hash = old_db.get('hdr_hash', '') if isinstance(old_db, dict) else ''

cur_files = {}
hdr_hasher = hashlib.md5()
changed_sources = set()

for root, dirs, files in os.walk('..'):
    if 'build_ios' in root or '.git' in root:
        continue
    for fn in sorted(files):
        p = os.path.join(root, fn).replace('\\', '/')
        try:
            data = open(p, 'rb').read()
            h = hashlib.md5(data).hexdigest()
            mt = os.path.getmtime(p)
            ext = os.path.splitext(fn)[1].lower()
            if ext in ('.h', '.hpp', '.inl') or fn == 'CMakeLists.txt':
                hdr_hasher.update(p.encode('utf-8') + b':' + h.encode('utf-8') + b'\n')
            if p not in old_files or old_files[p][0] != h:
                changed_sources.add(os.path.splitext(fn)[0])
            cur_files[p] = [h, mt]
        except Exception:
            pass

cur_hdr_hash = hdr_hasher.hexdigest()

if mode == 'restore':
    shutil.rmtree('build_ios/Release-iphoneos/M2WorldOnline.app', ignore_errors=True)
    if old_epoch != CACHE_EPOCH:
        print(f'[ios_cache_helper] Cache epoch updated ({old_epoch} -> {CACHE_EPOCH}). Performing clean rebuild of build_ios...')
        shutil.rmtree('build_ios/build/world_mobile.build', ignore_errors=True)
        shutil.rmtree('build_ios/build/XCBuildData', ignore_errors=True)
    elif old_hdr_hash != cur_hdr_hash:
        print('[ios_cache_helper] Header/CMakeLists change detected! Clearing world_mobile object files...')
        shutil.rmtree('build_ios/build/world_mobile.build', ignore_errors=True)
        shutil.rmtree('build_ios/build/XCBuildData', ignore_errors=True)
    else:
        now_ts = time.time() + 5.0
        for p, (h, mt) in cur_files.items():
            if p in old_files and old_files[p][0] == h:
                os.utime(p, (old_files[p][1], old_files[p][1]))
            else:
                os.utime(p, (now_ts, now_ts))
        if changed_sources and os.path.exists('build_ios/build'):
            for r, d, fls in os.walk('build_ios/build'):
                for f in fls:
                    if f.endswith('.o') and os.path.splitext(f)[0] in changed_sources:
                        try:
                            os.remove(os.path.join(r, f))
                        except Exception:
                            pass

if mode == 'save':
    os.makedirs('build_ios', exist_ok=True)
    saved_ts = time.time() - 3600.0
    for p in cur_files:
        cur_files[p][1] = saved_ts
    json.dump({'epoch': CACHE_EPOCH, 'hdr_hash': cur_hdr_hash, 'files': cur_files}, open(DB, 'w'))