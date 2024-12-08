<?php

// Spotify API Credentials
$clientId = 'CLIENT-ID HERE';
$clientSecret = 'CLIENT-SECRET HERE';
$refreshToken = 'REFRESH-TOKEN HERE';

// Access-Token cache
$accessTokenFile = __DIR__ . '/access_token.txt';

// Funktion, um neuen Access-Token zu holen
function getAccessToken($clientId, $clientSecret, $refreshToken) {
    $ch = curl_init('https://accounts.spotify.com/api/token');
    $data = [
        'grant_type' => 'refresh_token',
        'refresh_token' => $refreshToken
    ];

    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query($data));
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);

    $header = [
        'Authorization: Basic ' . base64_encode($clientId . ':' . $clientSecret)
    ];
    curl_setopt($ch, CURLOPT_HTTPHEADER, $header);

    $response = curl_exec($ch);
    $error = curl_error($ch);
    curl_close($ch);

    if ($error) {
        return null;
    }

    $json = json_decode($response, true);
    return $json['access_token'] ?? null;
}

// Aktuellen Access-Token holen (aus Datei, wenn vorhanden)
$accessToken = null;
if (file_exists($accessTokenFile)) {
    $accessToken = file_get_contents($accessTokenFile);
}

// Falls kein Access-Token vorhanden
if (empty($accessToken)) {
    $accessToken = getAccessToken($clientId, $clientSecret, $refreshToken);
    if ($accessToken) {
        file_put_contents($accessTokenFile, $accessToken);
    } else {
        header('Content-Type: application/json');
        echo json_encode(['error' => 'Could not refresh access token']);
        exit;
    }
}

// Nun den aktuell laufenden Track abfragen
$ch = curl_init('https://api.spotify.com/v1/me/player/currently-playing');
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, [
    'Authorization: Bearer ' . $accessToken
]);

$response = curl_exec($ch);
$httpStatus = curl_getinfo($ch, CURLINFO_HTTP_CODE);
$error = curl_error($ch);
curl_close($ch);

header('Content-Type: application/json');

if ($error) {
    echo json_encode(['error' => $error]);
    exit;
}

if ($httpStatus === 401) {
    // Access-Token ist wohl abgelaufen, neu holen und nochmal versuchen
    $accessToken = getAccessToken($clientId, $clientSecret, $refreshToken);
    if ($accessToken) {
        file_put_contents($accessTokenFile, $accessToken);
        // Noch ein Versuch:
        $ch = curl_init('https://api.spotify.com/v1/me/player/currently-playing');
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER, [
            'Authorization: Bearer ' . $accessToken
        ]);
        $response = curl_exec($ch);
        $error = curl_error($ch);
        curl_close($ch);

        if ($error) {
            echo json_encode(['error' => $error]);
            exit;
        }
    } else {
        echo json_encode(['error' => 'Could not refresh access token']);
        exit;
    }
}

// Kein Track gespielt oder kein Inhalt
if ($httpStatus === 204 || empty($response)) {
    echo json_encode(['playing' => false]);
    exit;
}

// Antwort ist JSON
$data = json_decode($response, true);

if (isset($data['item'])) {
    $trackName = $data['item']['name'] ?? '';
    $artists = [];
    if (isset($data['item']['artists'])) {
        foreach ($data['item']['artists'] as $artist) {
            $artists[] = $artist['name'];
        }
    }

    echo json_encode([
        'playing' => true,
        'track' => $trackName,
        'artists' => $artists
    ]);
} else {
    echo json_encode(['playing' => false]);
}
