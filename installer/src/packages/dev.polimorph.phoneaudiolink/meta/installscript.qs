function Component()
{
    // Load the custom UI form
    component.loaded.connect(this, Component.prototype.installerLoaded);

}

Component.prototype.installerLoaded = function() {
    // Add custom page with checkbox
    if (installer.addWizardPage(component, "CreateDesktopShortcutForm", QInstaller.InstallationFinished)) {
        var widget = gui.pageWidgetByObjectName("DynamicCreateDesktopShortcutForm");
        if (widget != null) {
            widget.windowTitle = "Shortcuts";
        }
    }
}

Component.prototype.createOperations = function()
{
    component.createOperations();
    if (systemInfo.productType === "windows") {

        // Create Start Menu entry
        component.addOperation("CreateShortcut", "@TargetDir@/PhoneAudioLink.exe", "@StartMenuDir@/PhoneAudioLink.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/icon.ico",
            "description=Open PhoneAudioLink");

        // Create Desktop shortcut (optional, based on checkbox)
        var widget = gui.pageWidgetByObjectName("DynamicCreateDesktopShortcutForm");
        if (widget != null) {
            var checkbox = widget.createDesktopShortcutCheckBox;
            if (checkbox.checked) {
                component.addOperation("CreateShortcut", 
                    "@TargetDir@/PhoneAudioLink.exe", 
                    "@DesktopDir@/PhoneAudioLink.lnk",
                    "workingDirectory=@TargetDir@", 
                    "iconPath=@TargetDir@/icon.ico",
                    "description=Open PhoneAudioLink");
            }
        }
    }
}
