// installscript.qs

function Component()
{
    if (!installer.isCommandLineInstance()) {
        var installPage = gui.pageWidgetByObjectName("PerformInstallationPage");
        if (installPage)
            installPage.entered.connect(updateInstallingText);

        var summaryPage = gui.pageWidgetByObjectName("ReadyForInstallationPage");
        if (summaryPage)
            summaryPage.entered.connect(updateSummaryText);
    }
}

function updateInstallingText()
{
    var page = gui.pageWidgetByObjectName("PerformInstallationPage");
    if (!page)
        return;

    if (page.MessageLabel)
        page.MessageLabel.setText(
            "Downloading and installing Genesis-X components. This may take a few moments..."
        );
}

function updateSummaryText()
{
    var page = gui.pageWidgetByObjectName("ReadyForInstallationPage");
    if (!page)
        return;

    var bytes = installer.requiredDiskSpace();
    var mb = Math.round(bytes / (1024 * 1024));

    if (page.MessageLabel) {
        var baseText = page.MessageLabel.text;
        page.MessageLabel.setText(
            baseText +
            "<br/><br/><b>Estimated size on disk:</b> " +
            mb + " MB"
        );
    }
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    var targetDir = installer.value("TargetDir");
    var archivePath = targetDir + "/genesisx-pre-release-v1.0.0-rc2.zip";

    var url = "https://github.com/KooijmanInc/Genesis-X/archive/refs/tags/v1.0.0-rc2.zip";

    var os = systemInfo.productType
    var ok = false;

    component.addOperation("Mkdir", targetDir);

    if (os === "windows") {
        ok = installer.execute("powershell", [
            "-NoProfile",
            "-ExecutionPolicy", "Bypass",
            "-Command",
            "Invoke-WebRequest -Uri '" + url + "' -OutFile '" + archivePath + "'"
        ]);
        component.addElevatedOperation(
            "Execute",
            "attrib",
            ["+s", "@TargetDir@", "/D"]
        );

        component.addElevatedOperation(
            "Execute",
            "attrib",
            ["+r", "@TargetDir@", "/D"]
        );
    } else {
        ok = installer.execute("curl", [
            "-L",
            "-o", archivePath,
            url
        ]);
    }

    if (!ok) {
        console.log("Genesis-X pre-release download failed from: " + url);
        installer.setStatus(Installer.Failure);
        return;
    }

    component.addOperation("Extract", archivePath, targetDir);

    component.addOperation("Delete", archivePath);
};
