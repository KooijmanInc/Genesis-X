function Component() {}

function applyFolderIcon(folderPath, iconPath)
{
    var os = systemInfo.productType;

    if (os === "windows") {
        var normalizedFolder = folderPath.replace(/\//g, "\\");
        var normalizedIcon   = iconPath.replace(/\//g, "\\");

        var folderForOps = folderPath;
        var iniFile = folderForOps + "\\desktop.ini";

        var iniContent =
            "[.ShellClassInfo]\r\n" +
            "IconResource=" + normalizedIcon + ",0\r\n" +
            "ConfirmFileOp=0\r\n" +
            "IconIndex=0";

        component.addOperation("Mkdir", folderForOps);

        // component.addOperation("Delete", iniFile);
        component.addOperation("AppendFile", iniFile, iniContent);

        var attribOk = installer.execute(
            "cmd",
            ["/C",
             "attrib +s +h \"" + iniFile + "\" /D"]
        );
        console.log("attrib result: " + (attribOk ? "OK" : "FAILED"));
    } else if (os === "osx") {

    } else {

    }
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    var os = systemInfo.productType;
    var targetDir = installer.value("TargetDir");

    if (os === "windows") {
        var iconDir  = targetDir + "/theme/icons";

        var iconPathForRoot = iconDir + "/GenDrive-GenX.ico";
        var subIconPath = iconDir + "/GenDrive-Folder.ico";
        var subIconDocumentPath = iconDir + "/GenDrive-Folder-Document.ico";
        var subIconCodePath = iconDir + "/GenDrive-Folder-Code.ico";

        applyFolderIcon(targetDir, iconPathForRoot);

        var codeIconWin     = subIconCodePath.replace(/\//g, "\\");
        var docIconWin      = subIconDocumentPath.replace(/\//g, "\\");
        var defaultIconWin  = subIconPath.replace(/\//g, "\\");

    } else if (os === "osx") {

    } else {

    }
};
