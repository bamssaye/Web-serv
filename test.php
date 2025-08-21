<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (isset($_FILES['file'])) {
        $target = __DIR__ . '/uploads/' . basename($_FILES['file']['name']);
        if (move_uploaded_file($_FILES['file']['tmp_name'], $target)) {
            echo "✅ Uploaded: " . htmlspecialchars($_FILES['file']['name']);
        } else {
            echo "❌ Upload failed.";
        }
    }
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Upload Test</title>
</head>
<body>
    <h2>Upload a file</h2>
    <form method="post" enctype="multipart/form-data">
        <input type="file" name="file">
        <button type="submit">Upload</button>
    </form>
</body>
</html>
