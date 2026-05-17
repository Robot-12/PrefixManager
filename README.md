<div align="center">

# 🏷️ Prefix Manager for Unreal Engine 5
### Automatic Asset Naming Convention Enforcer

![Unreal Engine 5](https://img.shields.io/badge/Unreal_Engine-5.0+-black?style=for-the-badge&logo=unrealengine)
![Status](https://img.shields.io/badge/Status-BETA-orange?style=for-the-badge)
![Author](https://img.shields.io/badge/Created_By-Robot_12-blue?style=for-the-badge)

</div>

---

## 👋 Introduction
Welcome to **Prefix Manager**! 🚀

This editor-only plugin for Unreal Engine 5 eliminates the tedious task of manually typing out prefixes and suffixes for your newly created assets. It dynamically applies your project's naming conventions directly in the Content Browser, effortlessly ensuring a clean and consistent project structure.

## ✨ Key Features
* **Intelligent Auto-Correction:** An Auto-Fix system that automatically appends missing prefixes and suffixes to assets upon creation, renaming, or even when an action is canceled (e.g., pressing Escape).
* **Live UI Validation:** Real-time Slate UI tooltips (red warnings) guide you while typing inline asset names, preventing invalid characters or incorrect formats before they happen.
* **Native Data Validation:** Integrates a custom `PrefixValidator` with Unreal Engine's native Data Validation subsystem to enforce strict naming rules automatically during the save and cook processes.
* **Automated Workflow:** Instantly prepends and appends text when you create a new asset based on fully customizable rules you define natively in the Project Settings.
* **Inheritance Support:** Optionally apply naming rules to derived child classes automatically (with smart type resolution for Blueprints and Widgets).
* **Developer Toggles:** Developers can easily disable any subsystem (Auto-Fix, UI Validation, Data Validation) locally in their Editor Preferences without affecting the rest of the team's project settings.

## 📦 Installation & Releases
You can download the pre-compiled, packaged versions of the plugin directly from the **[Releases](../../releases)** page.

1. Download the latest packaged `.zip` file that matches your specific **Unreal Engine version**.
2. Extract the `PrefixManager` folder and choose one of the following installation methods:
    * **Project Installation (Recommended):** Place the folder into your project's `Plugins` directory (e.g., `[YourProject]/Plugins/PrefixManager`). Create the `Plugins` folder if it doesn't exist.
    * **Engine Installation (Global):** Place the folder directly into your Unreal Engine installation directory (e.g., `[UE_Install_Directory]/Engine/Plugins/PrefixManager`). This makes the plugin available for all projects using that specific engine version.
3. Restart your Unreal Engine project and ensure the plugin is enabled via `Edit -> Plugins`.

## 📚 Documentation

### Configuring Project Rules (Global)
To set up the naming conventions for the entire team, the plugin uses Unreal's native settings architecture:
1. Go to `Edit -> Project Settings`.
2. Scroll down to the **Plugins** category and select **Prefix Manager (Rules)**.
3. Manage your naming rules in the custom table interface:
    * **Prefix:** The string added to the beginning of the asset name (e.g. `BP_`, `SM_`, `M_`, `WBP_`).
    * **Suffix:** The string appended to the end of the asset name (e.g. `_Skeleton`).
    * **Asset Class:** The target Unreal Engine class for this rule.
    * **Apply To Children:** Check this to make derived child classes inherit this naming rule. *(Note: Core structural types like standard Blueprints are locked to `true` by default).*
4. You can populate the table with standard Epic Games naming conventions instantly by clicking **Reset To Defaults**.

### Editor Preferences (Local Developer Override)
If a team member wants to customize their workflow or temporarily disable certain features without changing the global project settings, they can do so in their local config:
1. Go to `Edit -> Editor Preferences`.
2. Scroll down to the **Plugins** category and select **Prefix Manager (User)**.
3. Here you can toggle individual systems on or off, and tweak local settings:
    * **Enable Auto Prefixer:** Toggles the automatic injection of text when creating a new asset.
    * **Auto Prefix Delay:** Adjusts the delay (in seconds) before the text is injected. Increase this if the prefix doesn't appear immediately on slower machines.
    * **Enable Live UI Validation:** Toggles the real-time red warning tooltips.
    * **Enable Naming Auto-Correction:** Toggles the smart Auto-Fix system for renamed or canceled assets.
    * **Enable Validation:** Toggles the native Data Validation checking when saving files. *(Note: Disabled by default. Since the other automated systems already handle naming proactively, this strict validation can occasionally throw false-positive errors. It is intended as an optional layer for users who strictly require hard enforcement on save).*

## 🚀 Roadmap (Planned Features)
The plugin is currently in Beta, and the following features are planned for future releases:
* **Bulk Renaming Tool:** An option to mass-rename existing project assets to automatically align them with your newly defined rules.

## 🤝 Feedback
Feedback, suggestions, and pull requests are always welcome! Feel free to open an issue if you encounter any bugs or have ideas for new features.
