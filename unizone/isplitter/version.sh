#!/bin/sh
MAJOR=0
MINOR=2
PATCH=0
BUILD=b100
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
__EOF__
echo "	<string>$MAJOR.$MINOR.$PATCH$BUILD</string>"
cat << __EOF__
	<key>CFBundleSignature</key>
	<string>ttxt</string>
	<key>CFBundleExecutable</key>
	<string>ImageSplitter</string>
        <key>CFBundleVersion</key>
__EOF__
echo "	<string>$MAJOR.$MINOR.$PATCH$BUILD</string>"
cat << __EOF__
	<key>NOTE</key>
	<string>Please, do NOT change this file -- It was generated automatically.</string>
</dict>
</plist>
__EOF__