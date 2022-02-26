# signcenter\_tool<a name="EN-US_TOPIC_0000001086718894"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Constraints](#section119744591305)
-   [Usage](#section1312121216216)

## Introduction<a name="section11660541593"></a>

During system building, OpenHarmony signs system applications to ensure their integrity and source reliability. This module provides a binary tool \(**hapsigntoolv2.jar**\) for signing OpenHarmony system applications.

## Directory Structure<a name="section161941989596"></a>

```
/prebuilts/signcenter
├── NOTICE                     # Open-source NOTICE file
├── hapsigntool                # Signing tool
│   └── hapsigntoolv2.jar     # OpenHarmony signing tool
```

## Constraints<a name="section119744591305"></a>

Running environment: JDK 8

## Usage<a name="section1312121216216"></a>

The following is an example command for signing applications:

```
java -jar hapsigntoolv2.jar sign -mode localjks -privatekey "OpenHarmony Software Signature" -inputFile camera.hap -outputFile signed_camera.hap -signAlg SHA256withECDSA -keystore OpenHarmony.jks -keystorepasswd 123456 -keyaliaspasswd 123456 -profile camera_release.p7b -certpath OpenHarmony.cer -profileSigned 1
```

Descriptions of the fields in the preceding command:

```
hapsigntoolv2.jar: OpenHarmony signing tool
-mode: signing mode. The OpenHarmony signing key pair is stored in the local keystore file. The signing mode is localjks.
-privatekey: alias of the signing key pair
-inputFile: application to be signed
-outputFile: signed application
-signAlg: signing algorithm
-keystore: path of the keystore file
-keystorepasswd: password of the keystore file. The default password of OpenHarmony.jks is 123456.
-keyaliaspasswd: password of the signing key pair. The password of OpenHarmony Software Signature is 123456.
-profile: a file declaring the capabilities and permissions of the application
-certpath: path of the signing certificate
-profileSigned: whether the application profile is signed. Value 1 indicates that the profile is signed; value 0 indicates the opposite. The default value is 1.
```

