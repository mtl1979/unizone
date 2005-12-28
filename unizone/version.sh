#!/bin/sh
cat << __EOF__
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist SYSTEM "file://localhost/System/Library/DTDs/PropertyList.dtd">
<plist version="0.9">
<dict>
	<key>CFBundleIconFile</key>
	<string>application.icns</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
        <key>CFBundleGetInfoString</key>
	<string>Created by version.sh</string>
	<key>CFBundleSignature</key>
	<string>ttxt</string>
	<key>CFBundleExecutable</key>
	<string>Unizone</string>
        <key>CFBundleVersion</key>
__EOF__
MAJOR=`sed -n 's/.*kMajor = \([0-9]*\).*/\1/p' src/version.cpp`
MINOR=`sed -n 's/.*kMinor = \([0-9]*\).*/\1/p' src/version.cpp`
PATCH=`sed -n 's/.*kPatch = \([0-9]*\).*/\1/p' src/version.cpp`
BUILD=b`sed -n 's/.*kBuild = \([0-9]*\).*/\1/p' src/version.cpp`
echo "	<string>$MAJOR.$MINOR.$PATCH$BUILD</string>"
cat << __EOF__
	<key>NOTE</key>
	<string>Please, do NOT change this file -- It was generated automatically.</string>
</dict>
</plist>
__EOF__