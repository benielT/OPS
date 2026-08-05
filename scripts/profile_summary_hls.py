import os
import pandas as pd
import argparse
import re

def parse_tile_sizes(filename, tile):
    basename = filename.replace("_perf_profile.csv", "")
    parts = basename.split("_")

    if tile == 0:
        return None, None

    if len(parts) < tile:
        raise ValueError(
            f"Filename '{filename}' does not contain {tile} tile dimension(s) before '_perf_profile.csv'."
        )

    # Extract the last `tile` parts before the _perf_profile.csv suffix.
    tile_parts = parts[-tile:]
    if tile == 1:
        return int(tile_parts[0]), None
    return int(tile_parts[0]), int(tile_parts[1])


def generate_profile_summary(directory, tile):
    # List to store grid sizes and average main times
    summary_data = []

    # Iterate through all files in the directory
    for filename in os.listdir(directory):
        if filename.endswith("perf_profile.csv"):
            file_path = os.path.join(directory, filename)
            
            tile_x_size, tile_y_size = parse_tile_sizes(filename, tile)
            # Read the CSV file
            df = pd.read_csv(file_path)
            
            # Check if the last row starts with "args"
            if isinstance(df.iloc[-1, 0], str) and df.iloc[-1, 0].startswith("args"):
                args_line = ",".join(str(x) for x in df.iloc[-1])
                df = df.iloc[:-1]
            
            batch_size = None
            batched_batch = None

            if 'args_line' in locals():
                bsize_match = re.search(r'-bsize=(\d+)', args_line)
                batch_match = re.search(r'-batch=(\d+)', args_line)
                if bsize_match:
                    batch_size = int(bsize_match.group(1))
                if batch_match:
                    batched_batch = int(batch_match.group(1))
            
            # Extract unique grid sizes
            grid_x = int(df['grid_x'].iloc[0])
            grid_y = int(df['grid_y'].iloc[0])
            grid_z = int(df['grid_z'].iloc[0])
            
            # Calculate the average main_time
            avg_main_time = df['main_time'].mean()
            
            # Append the data to the summary list
            if batch_size is None:
                batch_size = 1
            
            record = {
                    "grid_x": grid_x,
                    "grid_y": grid_y,
                    "grid_z": grid_z,
                    "batch_size": batch_size,
                    "average_main_time": avg_main_time
            }
            if (tile_x_size != None):
                record["tile_x"] = tile_x_size
            if (tile_y_size != None ):
                record["tile_y"] = tile_y_size
            
            summary_data.append(record)
                

    # Create a DataFrame for the summary data
    summary_df = pd.DataFrame(summary_data)

    # Sort the DataFrame by grid sizes
    if (tile == 0):
        summary_df = summary_df.sort_values(by=["batch_size","grid_x", "grid_y", "grid_z"])
    elif (tile == 1):
        summary_df = summary_df.sort_values(by=["tile_x","grid_x", "grid_y", "grid_z"])
    elif (tile == 2):
        summary_df = summary_df.sort_values(by=["tile_x","tile_y","grid_x", "grid_y", "grid_z"])

    # Save the summary DataFrame to a new CSV file
    output_file = os.path.join(directory, "profile_summary.csv")
    summary_df.to_csv(output_file, index=False)

    print(f"Profile summary saved to {output_file}")

if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(description="Generate a profile summary CSV from a directory of CSV files.")
    parser.add_argument("-d", "--directory", type=str, help="Path to the directory containing the CSV files.")
    parser.add_argument("-t", "--tile", type=int, help="Tiling dimensions (optional). Default = 0", default=0)
    
    # Parse arguments
    args = parser.parse_args()
    
    # Generate the profile summary
    generate_profile_summary(args.directory, args.tile)