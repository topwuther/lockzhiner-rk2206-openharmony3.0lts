# Update App<a name="EN-US_TOPIC_0000001148414479"></a>

-   [Introduction](#section182mcpsimp)
-   [Directory Structure](#section190mcpsimp)
-   [Description](#section198mcpsimp)
-   [Repositories Involved](#section206mcpsimp)

## Introduction<a name="section182mcpsimp"></a>

The update app runs on the OHOS and provides an interactive GUI for users to perform version updates.

It provides the following functions:

1. Check for available update packages and display the check result.

2. Download the update package and display the download progress.

3. Trigger an update.

4. Obtain the version information after a version update.

## Directory Structure<a name="section190mcpsimp"></a>

```
base/update/app    # Update app code repository
├── entry          # Access to the update app code
│ └── src          # Update app code
└── gradle         # Configuration files
    └── wrapper    # Gradle configuration files
```

## Description<a name="section198mcpsimp"></a>

The update app is implemented by using JavaScript, and the service logic is implemented by using C++. The JavaScript app interacts with the service logic through NAPI.

For details about the NAPI implementation, see the following:

base/update/updateservice/client

## Repositories Involved<a name="section206mcpsimp"></a>

Update subsystem

**update\_app**

update\_updateservice

