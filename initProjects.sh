#!/bin/sh

# Folder containing your project targets
PROJECTS_DIR="Projects"

if [ ! -d "$PROJECTS_DIR" ]; then
    echo "Error: Directory '$PROJECTS_DIR' not found."
    exit 1
fi

# Loop through all subdirectories in Projects
for dir in "$PROJECTS_DIR"/*/; do
    # Remove trailing slash to get the folder name
    PROJECT_NAME=$(basename "$dir")

    echo "----------------------------------------"
    echo "Configuring: $PROJECT_NAME"
    echo "----------------------------------------"


    cmake -S . -B "$PROJECTS_DIR/$PROJECT_NAME" \
      -DTORQUE_APP_NAME="$PROJECT_NAME" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    if [ $? -eq 0 ]; then
        echo "Done: $PROJECT_NAME configured successfully."
    else
        echo "Error: Configuration failed for $PROJECT_NAME."
    fi
done
