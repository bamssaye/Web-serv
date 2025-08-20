<?php
// Set upload directory
$uploadDir = __DIR__ . '/uploads/';

// Create upload directory if it doesn't exist
if (!is_dir($uploadDir)) {
    mkdir($uploadDir, 0755, true);
}

// Handle image upload
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['image'])) {
    $file = $_FILES['image'];

    // Check for upload errors
    if ($file['error'] === UPLOAD_ERR_OK) {
        $fileName = basename($file['name']);
        $targetPath = $uploadDir . $fileName;

        // Move uploaded file to the uploads directory
        if (move_uploaded_file($file['tmp_name'], $targetPath)) {
            echo "✅ Image uploaded successfully: <a href='uploads/$fileName'>$fileName</a><br>";
            echo "<img src='uploads/$fileName' alt='Uploaded Image' style='max-width: 300px;'>";
        } else {
            echo "❌ Failed to move uploaded file.";
        }
    } else {
        echo "❌ File upload error: " . $file['error'];
    }
} else {
    // Show upload form
    ?>
    <!DOCTYPE html>
    <html>
    <body>
        <h2>Upload an Image</h2>
        <form method="POST" enctype="multipart/form-data">
            <input type="file" name="image" accept="image/*" required>
            <br><br>
            <button type="submit">Upload Image</button>
        </form>
    </body>
    </html>
    <?php
}
?>
