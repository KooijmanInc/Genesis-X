function Controller() {
}

Controller.prototype.IntroductionPageCallback = function() {
    var os = systemInfo.productType;

    var targetDir;
    if (os === "windows") {
        targetDir = "C:/Genesis-X";
        installerApplicationIcon = "logo.ico";
        installerWindowIcon = "logo.ico";
        logo = "watermark1.png";
        watermark = "watermark1.png";
    } else if (os === "osx") {
        targetDir = "/Library/Genesis-X";
        installerApplicationIcon = "logo.png";
        installerWindowIcon = "logo.png";
        logo = "watermark1.png";
        watermark = "watermark1.png";
    } else {
        targetDir = "/opt/Genesis-X";
        installerApplicationIcon = "logo.png";
        installerWindowIcon = "logo.png";
        logo = "watermark1.png";
        watermark = "watermark1.png";
    }

    installer.setValue("TargetDir", targetDir);

    var page = gui.pageWidgetByObjectName("IntroductionPage");
    if (!page || !page.MessageLabel)
        return;

    var text =
        "<h2 style='color:#00eaff; margin-bottom:8px;'>Welcome to Genesis-X</h2>"
        + "<p>Genesis-X is the foundation powering GenDrive, Lounge VX, and "
        + "next-generation Qt / QML applications.</p>"
        + "<p>This installer will:</p>"
        + "<ul style='margin-left:12px;'>"
        + "<li>Install Genesis-X core libraries and resources</li>"
        + "<li>Install optional tooling and developer assets</li>"
        + "<li>Install the GenDrive desktop theme</li>"
        + "</ul>"
        + "<p>Click <b>Next</b> to begin.</p>";

    page.MessageLabel.setText(text);
};
