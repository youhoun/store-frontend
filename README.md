# Sabai Books — Qt Desktop App

C++17 + Qt 6 Widgets desktop application.

## Google Login

The login page includes **Continue with Google** using Google OAuth 2.0 for an installed desktop application with PKCE. Authentication happens in the system browser.

### Setup
1. Create an OAuth 2.0 Client ID in Google Cloud Console with application type **Desktop app**.
2. Copy the Client ID.
3. In `mainwindow.cpp`, replace `PASTE_YOUR_GOOGLE_DESKTOP_CLIENT_ID_HERE` with your Client ID. The desktop OAuth flow uses PKCE, so no client secret is embedded in the application.
4. Rebuild and run.

The app requests only `openid email profile`. No server secret is required in the desktop client.

## Lectures
The supplied lecture PDFs are under `assets/lectures/`. CMake copies them next to the executable after each build.


### KHQR display
The payment dialog no longer displays the 03:00 countdown while scanning. The expiry is still handled internally so expired payments are not treated as valid.
