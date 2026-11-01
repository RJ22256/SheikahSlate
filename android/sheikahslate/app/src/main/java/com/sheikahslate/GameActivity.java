package com.sheikahslate;

// The game's activity (AndroidManifest.xml): SheikahSlateActivity's code under another name, so the launcher entries
// can be activity-aliases of it - Settings > Look > App icon switches which one is enabled (setAppIcon), and the
// original's alias keeps the name the activity had before (com.sheikahslate.SheikahSlateActivity): a home screen icon
// from an earlier version stays. The native code's JNI names stay SheikahSlateActivity's, the class that declares them.
public class GameActivity extends SheikahSlateActivity {
}
