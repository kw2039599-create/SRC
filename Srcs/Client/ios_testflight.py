import base64, glob, os, plistlib, shutil, subprocess, sys, time, zlib, struct
import jwt, requests

KEY_ID = "22DQZB7VR9"
ISSUER_ID = "9409b0ba-08b8-4acb-b6e1-55409b555de4"
TEAM_ID = "6A593PU2AF"
BUNDLE_ID = "com.m2world.online"
APP_NAME = "M2World Online"
APP_PATH = "build_ios/Release-iphoneos/M2WorldOnline.app"
