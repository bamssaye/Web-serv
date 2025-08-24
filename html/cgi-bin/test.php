<?php
// Directory to save files
$uploadDir = __DIR__ . "/uploads";
if (!is_dir($uploadDir)) {
    mkdir($uploadDir, 0777, true);
}

// Optional: get filename from query string ?name=myfile.txt
$filename = isset($_GET['name']) ? basename($_GET['name']) : ("upload_" . time());

// Build full path
$target = $uploadDir . "/" . $filename;

// Read raw input stream
$rawData = file_get_contents("php://input");

if ($rawData === false || strlen($rawData) === 0) {
    http_response_code(400);
    echo "❌ No data received.\n";
    exit;
}

// Save file
if (file_put_contents($target, $rawData) !== false) {
    http_response_code(201);
    echo "✅ File saved as: " . htmlspecialchars($filename) . "\n";
} else {
    http_response_code(500);
    echo "❌ Failed to save file.\n";
}
?>
