<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI File Upload Test</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f4f4f9; }
        .container { max-width: 600px; margin: auto; background: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1, h2 { color: #333; }
        form { margin-top: 20px; }
        input[type="file"] { border: 1px solid #ccc; padding: 10px; border-radius: 4px; width: 100%; }
        input[type="submit"] { background-color: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; margin-top: 10px; }
        input[type="submit"]:hover { background-color: #0056b3; }
        .message { padding: 15px; margin-top: 20px; border-radius: 5px; }
        .success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        code { background-color: #eef; padding: 2px 5px; border-radius: 4px; }
    </style>
</head>
<body>

<div class="container">
    <h1>CGI File Upload Test</h1>
    <p>Select a file to upload to the server.</p>

    <!-- The form must have enctype="multipart/form-data" -->
    <form action="test.php" method="post" enctype="multipart/form-data">
        Select file to upload:
        <input type="file" name="fileToUpload" id="fileToUpload">
        <input type="submit" value="Upload File" name="submit">
    </form>

    <?php
    // Check if the form was submitted
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
        
        // The directory where files will be saved.
        // IMPORTANT: This directory must exist and the server must have permission to write to it.
        $upload_dir = "uploads/";
        if (!is_dir($upload_dir)) {
            mkdir($upload_dir, 0755, true);
        }

        // Check if file was uploaded without errors
        if (isset($_FILES["fileToUpload"]) && $_FILES["fileToUpload"]["error"] == 0) {
            $filename = basename($_FILES["fileToUpload"]["name"]);
            $target_file = $upload_dir . $filename;

            // Move the file from the temporary directory to the final destination
            if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
                echo "<div class='message success'>";
                echo "<h2>File Uploaded Successfully!</h2>";
                echo "<p>The file <code>" . htmlspecialchars($filename) . "</code> has been uploaded.</p>";
                echo "<ul>";
                echo "<li><strong>Stored at:</strong> " . htmlspecialchars($target_file) . "</li>";
                echo "<li><strong>Size:</strong> " . $_FILES["fileToUpload"]["size"] . " bytes</li>";
                echo "<li><strong>Type:</strong> " . $_FILES["fileToUpload"]["type"] . "</li>";
                echo "</ul>";
                echo "</div>";
            } else {
                echo "<div class='message error'>";
                echo "<h2>Error!</h2>";
                echo "<p>Sorry, there was an error moving your uploaded file.</p>";
                echo "</div>";
            }
        } else {
            echo "<div class='message error'>";
            echo "<h2>Error!</h2>";
            $error_code = isset($_FILES["fileToUpload"]["error"]) ? $_FILES["fileToUpload"]["error"] : "Unknown";
            echo "<p>No file was uploaded or an error occurred. Error code: " . $error_code . "</p>";
            echo "<p>This might be because the file is larger than <strong>client_max_body_size</strong>.</p>";
            echo "</div>";
        }
    }
    ?>
</div>

</body>
</html>
