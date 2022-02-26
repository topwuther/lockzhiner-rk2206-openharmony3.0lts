# Photos<a name="EN-US_TOPIC_0000001177189635"></a>

-   [Introduction](#section443318452403)
    -   [Architecture](#section17686421134110)

-   [Directory Structure](#section7867113874118)
-   [Repositories Involved](#section229716044218)

## Introduction<a name="section443318452403"></a>

The Photos app is a pre-installed system app in the OpenHarmony system. It provides users with basic image browsing and album management functions, including viewing, moving, copying, deleting, and renaming images and videos.

### Architecture<a name="section17686421134110"></a>

![](figures/l2.png)

## Directory Structure<a name="section7867113874118"></a>

```
/applications/standard/photos/
├── figures                               # Architecture figures
├── entry                 
│   └── src
│       └── main
│           ├── config.json               # Global configuration file
│           ├── resources                 # Resource configuration files
│           └── js                        # JavaScript code
│               └── common                # Public resources
│               └── i18n                  # Internationalization
│               └── pages                 # Page code
├── signature                             # Certificate files
├── LICENSE                               # License files
```

## Repositories Involved<a name="section229716044218"></a>

System apps

**applications\_standard\_photos**

