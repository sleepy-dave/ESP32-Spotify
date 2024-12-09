> [!NOTE]
> Work in progress!
# Spotify Track Display with ESP32 and SSD1306 OLED
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

This project displays the currently playing Spotify track on an SSD1306 OLED display, controlled by an ESP32 (AZ-Delivery DevKit V4). The system connects to a Wi-Fi network, regularly fetches data from the Spotify API, and shows the track and artist names in a scrolling text display. Additionally, a music icon is displayed for visual enhancement. Developed with PlatformIO and the Arduino framework, the project utilizes libraries such as Adafruit GFX, Adafruit SSD1306, and ArduinoJson for efficient rendering and data processing.

## Features
+ **Wi-Fi Connection**: Stable connection to your Wi-Fi network.
+ **Spotify Integration**: Retrieves the currently playing track from the Spotify API.
+ **OLED Display**: Shows the track name and artist on an SSD1306 OLED display.
+ **Scrolling Text Display**: Smooth scrolling of text for better readability.
+ **Music Icon**: Displays a music icon alongside the track text.
+ **Real-time Updates**: Updates the display every second with the latest track information.

## Hardware
+ **ESP32 Development Board**: AZ-Delivery DevKit V4
+ **SSD1306 OLED Display**: 128x32 pixels
+ **Connection Cables**: For I2C communication between ESP32 and OLED
+ **Power Supply**: USB cable or suitable power source for the ESP32 board

# Installation

## ESP32 Setup
1. **Clone the Repository:**
   ```
   git clone https://github.com/sleepy-dave/ESP32-Spotify.git
   cd ESP32-Spotify
   ```
3. **Insert Your Credentials:**
   - Open the `spotifyAPI.php` file and add your Spotify Client ID and Client Secret.
4. **Connect the Display:**
   - Connect the SSD1306 OLED display to the ESP32 using the I2C pins (SDA and SCL).
5. **Flash the Code:**
   - Use PlatformIO or the Arduino IDE to upload the code to the ESP32 board.

## Spotify (For Developers)
1. **Create a Spotify App:**
   - Go to the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard/) and create a new app.
2. **Configure Redirect URI:**
   - Under "Redirect URIs," add the URL to your `callback.php`, e.g., `https://your-domain.com/spotify/callback.php`.
3. **Select APIs:**
   - Choose **Web API** from the available APIs, as no other APIs are used.
4. **Save Client ID and Secret:**
   - Navigate to your app's dashboard to find your "Client ID" and "Client Secret." Save these for the next steps.
5. **Generate the Authorization URL:**
   - Construct the authorization URL using your Client ID and Redirect URI. Replace your-client-id and your-redirect-uri with your actual values.

     `https://accounts.spotify.com/authorize?client_id=your-client-id&response_type=code&redirect_uri=your-redirect-uri&scope=user-read-currently-playing`

     (Note: Ensure that the redirect_uri is URL-encoded. You can use online tools or programming libraries to encode it properly.)

## Webserver Setup
1. **Prepare Your Webserver:**
   - Ensure you have a functioning webserver with PHP support (tested with PHP 8.1; other versions may also work).
2. **Install PHP Extensions:**
   - Ensure the following PHP extensions are installed and enabled in your `php.ini`:
     - **php-curl**
     - **php-json**
   - **Installation on Debian/Ubuntu:**
     ```
     sudo apt-get update
     sudo apt-get install php-curl php-json
     ```
3. **Upload PHP Files:**
   - Upload the two PHP files (`spotifyAPI.php` and `callback.php`) to your web root directory.
4. **Insert Client ID and Secret:**
   - Open the `spotifyAPI.php` file and add your Spotify Client ID and Client Secret.
5. **Create Token Files:**
   - Create two empty files in the same directory:
     - `refresh_token.txt`
     - `access_token.txt`
   - Set the correct permissions:
     ```
     chmod 600 refresh_token.txt access_token.txt
     chown www-data:www-data refresh_token.txt access_token.txt   # Adjust the user and group to match your webserver
     ```
6. **Obtain Refresh Token:**
   - Visit the authorization URL for your `callback.php` to obtain and save the Refresh Token in `refresh_token.txt`.

# Usage
- Once everything is set up, your ESP32 should regularly fetch the currently playing Spotify track and display it on the OLED screen.
- Ensure your ESP32 is connected to the same network as your webserver for seamless communication.

# Troubleshooting
- **Tokens Not Automatically Updating:**
  - Verify that the Refresh Token is correctly stored in `refresh_token.txt`.
  - Ensure the PHP extensions `curl` and `json` are enabled.
  - Check the file permissions of the token files.
- **OLED Display Issues:**
  - Check the I2C connections between the ESP32 and OLED display.
  - Ensure the correct I2C address is used in the code (usually `0x3C`).

# License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

# Acknowledgements
Thanks to the developers of the used libraries and the community for their support!
