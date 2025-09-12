# Resource Files

This directory contains all the assets used by the Puzzle Game application.

## 📁 Directory Structure

```
res/
├── img/           # Image resources
│   ├── 1.jpg      # Built-in puzzle image 1
│   ├── 2.jpg      # Built-in puzzle image 2
│   ├── ...        # More puzzle images
│   ├── blank.png  # Empty/transparent piece
│   └── imageshow.jpg # Reference image display
├── sounds/        # Sound effects
│   ├── correct.mp3    # Piece correctly placed
│   ├── move.mp3       # Piece moved
│   └── rotate.mp3     # Piece rotated
└── img.qrc       # Qt resource configuration
```

## 🖼️ Images

### Puzzle Images (1.jpg - 12.jpg)
- Format: JPEG/PNG
- Resolution: Various (recommended 800x600 minimum)
- Usage: Built-in puzzle images that can be selected by users
- Licensing: Ensure all images have appropriate licenses for redistribution

### UI Elements
- **blank.png**: Transparent image used for empty puzzle slots
- **imageshow.jpg**: Default reference image shown in help dialog

### Adding New Images
1. Place image files in `res/img/`
2. Update `res/img.qrc` to include new files
3. Use `:/img/filename` prefix in code to access images
4. Test image loading and display

## 🔊 Sounds

### Sound Effects
- **correct.mp3**: Played when puzzle piece is placed correctly
- **move.mp3**: Played when puzzle piece is moved
- **rotate.mp3**: Played when puzzle piece is rotated

### Audio Specifications
- Format: MP3 (recommended for compatibility)
- Duration: Short clips (1-3 seconds)
- Volume: Normalized to avoid sudden loud sounds

### Adding New Sounds
1. Place sound files in `res/sounds/`
2. Update resource references in code
3. Test audio playback on all supported platforms

## ⚙️ Resource Configuration

### Qt Resource System
The project uses Qt's resource system (`img.qrc`) to embed assets directly into the executable:
- No external file dependencies
- Cross-platform compatibility
- Simplified deployment

### Accessing Resources in Code
```cpp
// Images
QPixmap(":/img/1.jpg");
QImage(":/img/blank.png");

// Sounds
QMediaPlayer::setSource(QUrl(":/sounds/correct.mp3"));
```

## 📦 Optimization

### Image Optimization
- Compress images to reduce executable size
- Use appropriate resolution for puzzle pieces
- Consider progressive JPEG for large images
- Remove unnecessary metadata

### Audio Optimization
- Use appropriate bitrate (128-192 kbps for MP3)
- Normalize audio levels
- Remove silent portions
- Consider shorter loops for background music

## 🔧 Maintenance

### Adding New Assets
1. Check file size and format compatibility
2. Test on all target platforms
3. Update documentation
4. Commit with descriptive messages

### Removing Assets
1. Remove from filesystem
2. Update resource configuration
3. Update code references
4. Test functionality

## 📄 Licensing

Ensure all assets have compatible licenses:
- **Images**: Must allow redistribution and modification
- **Sounds**: Must allow redistribution and use in commercial applications
- **Custom assets**: Include license information in this directory

## 🚀 Deployment

Resources are automatically included in the build process:
- No separate asset deployment needed
- Single executable distribution
- Cross-platform asset handling