import base64, json, os, plistlib, shutil, subprocess, sys, time, zlib, struct
import jwt, requests
KEY_ID = ~22DQZB7VR9~
ISSUER_ID = ~9409b0ba-08b8-4acb-b6e1-55409b555de4~
TEAM_ID = ~6A593PU2AF~
BUNDLE_ID = ~com.m2world.online~
APP_NAME = ~M2World Online~
APP_PATH = ~build_ios/Release-iphoneos/M2WorldOnline.app~
def make_png(w, h):
    raw = b~~
    for y in range(h):
        row = bytearray([0])
        for x in range(w):
            bdr = (x _LT_ w//16 or x _GT_= w - w//16 or y _LT_ h//16 or y _GT_= h - h//16)
            row.extend((218, 165, 32) if bdr else (19, 27, 40))
        raw += bytes(row)
    def chk(t, d):
        return struct.pack(~_GT_I~, len(d)) + t + d + struct.pack(~_GT_I~, zlib.crc32(t + d) _AMP_ 0xffffffff)
    ihdr = struct.pack(~_GT_IIBBBBB~, w, h, 8, 2, 0, 0, 0)
    return b~\x89PNG\r\n\x1a\n~ + chk(b~IHDR~, ihdr) + chk(b~IDAT~, zlib.compress(raw, 9)) + chk(b~IEND~, b~~)
def prepare_bundle():
    os.makedirs('Assets.xcassets/AppIcon.appiconset', exist_ok=True)
    icons = [('icon120.png',120,'iphone','60x60','2x'),('icon180.png',180,'iphone','60x60','3x'),('icon152.png',152,'ipad','76x76','2x'),('icon167.png',167,'ipad','83.5x83.5','2x'),('icon1024.png',1024,'ios-marketing','1024x1024','1x')]
    imgs = []
    for fn, sz, idiom, size_s, scale in icons:
        open(f'Assets.xcassets/AppIcon.appiconset/{fn}', 'wb').write(make_png(sz, sz))
        imgs.append({'filename': fn, 'idiom': idiom, 'size': size_s, 'scale': scale})
    open('Assets.xcassets/AppIcon.appiconset/Contents.json', 'w').write(json.dumps({'images': imgs, 'info': {'version': 1, 'author': 'xcode'}}))
    subprocess.check_call(['xcrun', 'actool', 'Assets.xcassets', '--compile', APP_PATH, '--platform', 'iphoneos', '--minimum-deployment-target', '14.0', '--app-icon', 'AppIcon', '--output-partial-info-plist', 'partial_icons.plist'])
    sb = '<?xml version="1.0" encoding="UTF-8"?><document type="com.apple.InterfaceBuilder3.CocoaTouch.Storyboard.XIB" version="3.0" toolsVersion="21507" targetRuntime="iOS.CocoaTouch" propertyAccessControl="none" useAutolayout="YES" launchScreen="YES" useTraitCollections="YES" useSafeAreas="YES" colorMatched="YES" initialViewController="01J-lp-oVM"><device id="retina6_12" orientation="landscape" appearance="light"/><dependencies><plugIn identifier="com.apple.InterfaceBuilder.IBCocoaTouchPlugin" version="21505"/><capability name="Safe area layout guides" minToolsVersion="9.0"/><capability name="documents saved in the Xcode 8 format" minToolsVersion="8.0"/></dependencies><scenes><scene sceneID="EHf-IW-A2E"><objects><viewController id="01J-lp-oVM" sceneMemberID="viewController"><view key="view" contentMode="scaleToFill" id="Ze5-6b-2t3"><rect key="frame" x="0.0" y="0.0" width="852" height="393"/><autoresizingMask key="autoresizingMask" widthSizable="YES" heightSizable="YES"/><viewLayoutGuide key="safeArea" id="6Tk-OE-BBY"/><color key="backgroundColor" red="0.043" green="0.059" blue="0.090" alpha="1" colorSpace="custom" customColorSpace="sRGB"/></view></viewController><placeholder placeholderIdentifier="IBFirstResponder" id="iYj-Kq-Ea1" userLabel="First Responder" sceneMemberID="firstResponder"/></objects></scene></scenes></document>'
    open('LaunchScreen.storyboard', 'w').write(sb)
    subprocess.check_call(['xcrun', 'ibtool', '--compile', f'{APP_PATH}/LaunchScreen.storyboardc', 'LaunchScreen.storyboard'])
    ip_path = f'{APP_PATH}/Info.plist'
    subprocess.check_call(['plutil', '-convert', 'xml1', ip_path])
    pl = plistlib.load(open(ip_path, 'rb'))
    if os.path.exists('partial_icons.plist'): pl.update(plistlib.load(open('partial_icons.plist', 'rb')))
    pl['CFBundleDisplayName'] = APP_NAME
    pl['CFBundleName'] = 'M2WorldOnline'
    pl['CFBundleExecutable'] = 'M2WorldOnline'
    pl['CFBundleIdentifier'] = BUNDLE_ID
    pl['CFBundleShortVersionString'] = '1.0.0'
    pl['CFBundleVersion'] = str(os.environ.get('GITHUB_RUN_NUMBER', '1'))
    pl['UILaunchStoryboardName'] = 'LaunchScreen'
    pl['UIRequiresFullScreen'] = True
    pl['ITSAppUsesNonExemptEncryption'] = False
    pl['UIDeviceFamily'] = [1, 2]
    orient = ['UIInterfaceOrientationLandscapeLeft', 'UIInterfaceOrientationLandscapeRight', 'UIInterfaceOrientationPortrait', 'UIInterfaceOrientationPortraitUpsideDown']
    pl['UISupportedInterfaceOrientations'] = orient
    pl['UISupportedInterfaceOrientations' + 'ipad'] = orient
    plistlib.dump(pl, open(ip_path, 'wb'))
    subprocess.check_call(['plutil', '-convert', 'binary1', ip_path])
def get_p8():
    raw = os.environ.get('ASC_PRIVATE_KEY', '').strip()
    if not raw: return None
    hdr_b, hdr_e = '-----BEGIN ' + 'PRIVATE KEY-----', '-----END ' + 'PRIVATE KEY-----'
    body = raw.replace(hdr_b, '').replace(hdr_e, '').replace('\r', '').replace('\n', '').replace(' ', '')
    lines = [body[i:i+64] for i in range(0, len(body), 64)]
    return hdr_b + '\n' + '\n'.join(lines) + '\n' + hdr_e + '\n'
def asc_headers(p8):
    now = int(time.time())
    tok = jwt.encode({'iss': ISSUER_ID, 'iat': now - 30, 'exp': now + 1100, 'aud': 'appstoreconnect-v1'}, p8, algorithm='ES256', headers={'kid': KEY_ID, 'typ': 'JWT'})
    return {'Authorization': f'Bearer {tok}', 'Content-Type': 'application/json'}
def sign_and_upload(p8):
    hdr = asc_headers(p8)
    base = 'https://api.appstoreconnect.apple.com/v1'
    r = requests.get(f'{base}/bundleIds?filter[identifier]={BUNDLE_ID}', headers=hdr).json()
    bid = next((x['id'] for x in r.get('data', []) if x['attributes']['identifier'] == BUNDLE_ID), None)
    if not bid:
        cr = requests.post(f'{base}/bundleIds', headers=hdr, json={'data': {'type': 'bundleIds', 'attributes': {'identifier': BUNDLE_ID, 'name': 'M2World Online', 'platform': 'IOS'}}}).json()
        bid = cr['data']['id']
    cdir = os.path.expanduser(''/.ios_signing')
    os.makedirs(cdir, exist_ok=True)
    kfile, cfile, idfile = f'{cdir}/dist.key', f'{cdir}/dist.cer', f'{cdir}/cert.id'
    cid = open(idfile).read().strip() if os.path.exists(idfile) else None
    if cid and os.path.exists(kfile) and os.path.exists(cfile):
        if requests.get(f'{base}/certificates/{cid}', headers=hdr).status_code != 200: cid = None
    if not cid:
        subprocess.check_call(['openssl', 'genrsa', '-out', kfile, '2048'])
        subprocess.check_call(['openssl', 'req', '-new', '-key', kfile, '-out', f'{cdir}/dist.csr', '-subj', '/CN=M2World Online/O=M2World/C=US'])
        csr = ''.join(ln for ln in open(f'{cdir}/dist.csr').read().splitlines() if not ln.startswith('-----'))
        cr = requests.post(f'{base}/certificates', headers=hdr, json={'data': {'type': 'certificates', 'attributes': {'certificateType': 'IOS_DISTRIBUTION', 'csrContent': csr}}})
        if cr.status_code >= 400:
            ex = requests.get(f'{base}/certificates?filter[certificateType]=IOS_DISTRIBUTION', headers=hdr).json().get('data', [])
            if ex:
                old_id = ex[0]['id']
                requests.delete(f'{base}/certificates/{old_id}', headers=hdr)
                cr = requests.post(f'{base}/certificates', headers=hdr, json={'data': {'type': 'certificates', 'attributes': {'certificateType': 'IOS_DISTRIBUTION', 'csrContent': csr}}})
        cdata = cr.json()['data']
        cid = cdata['id']
        open(cfile, 'wb').write(base64.b64decode(cdata['attributes']['certificateContent']))
        open(idfile, 'w').write(cid)
    pr = requests.get(f'{base}/profiles?filter[name]=M2WorldOnline_AppStore', headers=hdr).json()
    for oldp in pr.get('data', []):
        oid = oldp['id']
        requests.delete(f'{base}/profiles/{oid}', headers=hdr)
    pcreate = requests.post(f'{base}/profiles', headers=hdr, json={'data': {'type': 'profiles', 'attributes': {'name': 'M2WorldOnline_AppStore', 'profileType': 'IOS_APP_STORE'}, 'relationships': {'bundleId': {'data': {'type': 'bundleIds', 'id': bid}}, 'certificates': {'data': [{'type': 'certificates', 'id': cid}]}}}}).json()
    mprov = f'{APP_PATH}/embedded.mobileprovision'
    open(mprov, 'wb').write(base64.b64decode(pcreate['data']['attributes']['profileContent']))
    cms = subprocess.check_output(['security', 'cms', '-D', '-i', mprov])
    plistlib.dump(plistlib.loads(cms)['Entitlements'], open('entitlements.plist', 'wb'))
    subprocess.call(['security', 'create-keychain', '-p', '1234', 'build.keychain'])
    subprocess.check_call(['security', 'default-keychain', '-s', 'build.keychain'])
    subprocess.check_call(['security', 'unlock-keychain', '-p', '1234', 'build.keychain'])
    subprocess.check_call(['security', 'set-keychain-settings', '-t', '3600', '-u', 'build.keychain'])
    open('wwdr.cer', 'wb').write(requests.get('https://www.apple.com/certificateauthority/AppleWWDRCAG3.cer').content)
    subprocess.check_call(['security', 'import', 'wwdr.cer', '-k', 'build.keychain', '-A'])
    subprocess.check_call(['security', 'import', cfile, '-k', 'build.keychain', '-A'])
    subprocess.check_call(['security', 'import', kfile, '-k', 'build.keychain', '-P', '', '-T', '/usr/bin/codesign', '-T', '/usr/bin/security'])
    subprocess.check_call(['security', 'set-key-partition-list', '-S', 'apple-tool:,apple:,codesign:', '-s', '-k', '1234', 'build.keychain'])
    sha1 = subprocess.check_output(['openssl', 'x509', '-inform', 'DER', '-in', cfile, '-noout', '-fingerprint', '-sha1']).decode().split('=')[1].replace(':', '').strip()
    subprocess.check_call(['/usr/bin/codesign', '--force', '--sign', sha1, '--entitlements', 'entitlements.plist', '--timestamp=none', APP_PATH])
    shutil.rmtree('Payload', ignore_errors=True)
    os.makedirs('Payload', exist_ok=True)
    shutil.copytree(APP_PATH, 'Payload/M2WorldOnline.app')
    subprocess.check_call(['/usr/bin/ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', 'Payload', 'M2WorldOnline.ipa'])
    apps = requests.get(f'{base}/apps?filter[bundleId]={BUNDLE_ID}', headers=hdr).json().get('data', [])
    if not apps:
        print('=== ACTION REQUIRED: Go to https://appstoreconnect.apple.com/apps -> Add App -> Select com.m2world.online ===')
        return
    adir = os.path.expanduser(''/.appstoreconnect/private' + '_keys')
    os.makedirs(adir, exist_ok=True)
    open(f'{adir}/AuthKey_{KEY_ID}.p8', 'w').write(p8)
    subprocess.check_call(['xcrun', 'altool', '--upload-app', '--type', 'ios', '-f', 'M2WorldOnline.ipa', '--apiKey', KEY_ID, '--apiIssuer', ISSUER_ID])
if __name__ == '__main__':
    prepare_bundle()
    p8 = get_p8()
    if p8: sign_and_upload(p8)
