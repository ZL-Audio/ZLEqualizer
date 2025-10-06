import os

def modify_svg_files():
    """
    Scans the current directory for .svg files, adds a <rect> element
    after the opening <svg> tag, and replaces the color 'black' with '#000000'.
    """
    # Get the current working directory
    current_directory = os.getcwd()
    print(f"Scanning for SVG files in: {current_directory}")

    # The line to be inserted
    rect_line = '<rect x="0" y="0" width="100%" height="100%" fill="none" />\n'
    
    # Iterate over all files in the current directory
    for filename in os.listdir(current_directory):
        if filename.endswith(".svg"):
            file_path = os.path.join(current_directory, filename)
            print(f"Processing: {filename}")
            
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    lines = f.readlines()
                
                # Flags to track modifications
                made_rect_change = False
                made_color_change = False

                # --- Modification 1: Add <rect> element ---
                svg_tag_index = -1
                for i, line in enumerate(lines):
                    if '<svg' in line:
                        svg_tag_index = i
                        break
                
                if svg_tag_index != -1:
                    # Check if the rect line already exists to avoid duplicates
                    if rect_line.strip() not in (l.strip() for l in lines):
                        lines.insert(svg_tag_index + 1, rect_line)
                        made_rect_change = True

                # --- Modification 2: Replace 'black' with '#000000' ---
                processed_lines = []
                for line in lines:
                    new_line = line.replace('black', '#000000')
                    if new_line != line:
                        made_color_change = True
                    processed_lines.append(new_line)

                # --- Write back to file if any changes were made ---
                if made_rect_change or made_color_change:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.writelines(processed_lines)
                    
                    # Report what was done
                    if made_rect_change and made_color_change:
                        print(f"  -> Added <rect> and replaced 'black' in {filename}")
                    elif made_rect_change:
                        print(f"  -> Successfully added <rect> element to {filename}")
                    elif made_color_change:
                        print(f"  -> Successfully replaced 'black' in {filename}")
                else:
                    print(f"  -> No changes needed for {filename}. Skipping.")

            except Exception as e:
                print(f"  -> An error occurred while processing {filename}: {e}")

    print("\nScript finished.")

if __name__ == "__main__":
    modify_svg_files()

