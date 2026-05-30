
import sys
import os
import json
import random
import argparse
from jinja2 import Environment, FileSystemLoader, Undefined

top_cpp_template = "./top.cpp.j2"
top_hpp_template = "./top.hpp.j2"
testbench_cpp_template = "./main.cpp.j2"

def get_ops_path():
    ops_path = os.environ["OPS_INSTALL_PATH"]

    if ops_path is None:
        raise EnvironmentError("Critical Error: OPS_INSTALL_PATH is not defined\n")
    
    return ops_path

def read_constrains():
    """Read JSON file './generator_parameter_constrains.json' and return as dict.
    Raises FileNotFoundError if the file does not exist or ValueError on parse error.
    """
    path = os.path.join(os.path.dirname(__file__), "generator_parameter_constrains.json")
    if not os.path.exists(path):
        raise FileNotFoundError(f"Required file not found: {path}")
    with open(path, "r", encoding="utf-8") as f:
        try:
            return json.load(f)
        except json.JSONDecodeError as e:
            raise ValueError(f"Invalid JSON in {path}: {e}")


def _is_power_of_two(value):
    return isinstance(value, int) and value > 0 and (value & (value - 1)) == 0


def _next_power_of_two(value):
    if value <= 0:
        return 1
    return 1 << ((value - 1).bit_length())


def generate_design_parameter(constrains, random_seed = None):
    """Generate random design parameters from constraints dict.

    Expects keys:
      MEM_DATA_WIDTH: list of valid mem data widths
      AXIS_DATA_WIDTH: list of valid axis data widths
      TILE_BANKS: list of valid tile bank values
      BURST_SIZE_MIN: integer minimum burst size
      BURST_SIZE_MAX: integer maximum burst size

    Additional constraints:
      axis_data_width must be <= mem_data_width
      burst_size must be a power of 2
    
    Args:
      random_seed: optional seed for random selection. If None, a random seed is generated
                   and included in the output for reproducibility.
    
    Returns:
      dict with keys: mem_data_width, axis_data_width, tile_banks, burst_size, ii, seed
    """
    try:
        mem_data_width = constrains["MEM_DATA_WIDTH"]
        axis_data_width = constrains["AXIS_DATA_WIDTH"]
        data_width = constrains["DATA_WIDTH"]
        tile_options = constrains["TILE_BANKS"]
        burst_min = constrains["BURST_SIZE_MIN"]
        burst_max = constrains["BURST_SIZE_MAX"]
        max_grid_size_x = constrains["MAX_GRID_SIZE_X"]
        max_grid_size_y = constrains["MAX_GRID_SIZE_Y"]
        max_grid_size_z = constrains["MAX_GRID_SIZE_Z"]
        min_grid_size_x = constrains["MIN_GRID_SIZE_X"]
        min_grid_size_y = constrains["MIN_GRID_SIZE_Y"]
        min_grid_size_z = constrains["MIN_GRID_SIZE_Z"]
        max_axi_depth = constrains["MAX_AXI_DEPTH"]
        type_name = constrains["TYPE"]
        stencil_d_max = constrains["STENCIL_D_MAX"]
        valid_tile_sizes = constrains["VALID_TILE_SIZES"]
        
    except KeyError as exc:
        raise KeyError(f"Missing required constraint: {exc}")

    if not isinstance(mem_data_width, list) or not isinstance(axis_data_width, list) or not isinstance(tile_options, list) or not isinstance(valid_tile_sizes, list):
        raise ValueError("MEM_DATA_WIDTH, AXIS_DATA_WIDTH, TILE_BANKS and VALID_TILE_SIZES must be lists")
    if not mem_data_width or not axis_data_width or not tile_options:
        raise ValueError("MEM_DATA_WIDTH, AXIS_DATA_WIDTH, and TILE_BANKS must not be empty")
    if not isinstance(burst_min, int) or not isinstance(burst_max, int) or not isinstance(max_grid_size_x, int) or not isinstance(max_grid_size_y, int) or not isinstance(max_grid_size_x, int) or not isinstance(min_grid_size_x, int) or not isinstance(min_grid_size_y, int) or not isinstance(min_grid_size_y, int) or not isinstance(stencil_d_max, int) or not isinstance(max_axi_depth, int):
        raise ValueError("BURST_SIZE_MIN, BURST_SIZE_MAX, MAX_GRID_SIZE_X, MAX_GRID_SIZE_Y, MAX_GRID_SIZE_Z, MAX_AXI_DEPTH, MIN_GRID_SIZE_X, MIN_GRID_SIZE_Y, MIN_GRID_SIZE_Z, and STENCIL_D_MAX must be integers")
    if burst_min > burst_max:
        raise ValueError("BURST_SIZE_MIN must be less than or equal to BURST_SIZE_MAX")
        
    # get the OPS_INSTALL path from environment variables
    ops_install_path = get_ops_path()
    
    # Handle random seed: generate one if not provided
    if random_seed is None:
        random_seed = random.randint(0, 2**31 - 1)
    
    random.seed(random_seed)

    mem_data_width_sorted = sorted(mem_data_width)
    axis_data_width_sorted = sorted(axis_data_width)
    tile_options_sorted = sorted(tile_options)

    # Find all valid mem_data_width and axis_data_width pairs
    valid_mem_data_width = []
    for mem_value in mem_data_width_sorted:
        valid_axes = [axis for axis in axis_data_width_sorted if axis <= mem_value]
        if valid_axes:
            valid_mem_data_width.append((mem_value, valid_axes))
    
    if not valid_mem_data_width:
        raise ValueError("No valid MEM_DATA_WIDTH and AXIS_DATA_WIDTH pair found under constraints")
    
    # Randomly select a mem_data_width and corresponding axis_data_width
    mem_data_width, valid_axes = random.choice(valid_mem_data_width)
    axis_data_width = random.choice(valid_axes)

    # Randomly select tile_banks
    tile_banks = random.choice(tile_options_sorted)

    # Find all valid power-of-2 burst sizes within range
    valid_burst_sizes = []
    burst_size = _next_power_of_two(burst_min)
    while burst_size <= burst_max:
        valid_burst_sizes.append(burst_size)
        burst_size *= 2
    
    if not valid_burst_sizes:
        raise ValueError("No valid power-of-two burst size found within constraints")
    
    # Randomly select a burst_size
    burst_size = random.choice(valid_burst_sizes)

    # Calculate ii (initiation interval)
    ii = mem_data_width // axis_data_width
    
    # Calculating vector factor
    mem_vector_factor = int(mem_data_width / data_width)
    
    # Adjusting max_grid_size_x based on mem_vector_factor
    max_grid_size_x = int((max_grid_size_x + mem_vector_factor - 1) / mem_vector_factor) *  mem_vector_factor
    max_axi_depth_calculated = int((max_grid_size_x * max_grid_size_y * max_grid_size_z / mem_vector_factor))
    if max_axi_depth < max_axi_depth_calculated :
        old_val = max_axi_depth
        max_axi_depth = _next_power_of_two(max_axi_depth_calculated)
        print(f"[WARNING] the MAX_AXI_DEPTH ({old_val}) is not adequate to accomodate max grid: ({max_grid_size_x}, {max_grid_size_y}, {max_grid_size_z}). Adjusting max_axi_depth to: {max_axi_depth}")
        
    return {
        "mem_data_width": mem_data_width,
        "axis_data_width": axis_data_width,
        "mem_vector_factor" : mem_vector_factor,
        "tile_banks": tile_banks,
        "burst_size": burst_size,
        "ii": ii,
        "max_axi_depth" : max_axi_depth,
        "seed": random_seed,
        "ops_install_path": ops_install_path,
        "data_width" : data_width,
        "type" : type_name,
        "max_grid_size_x" : max_grid_size_x,
        "max_grid_size_y" : max_grid_size_y,
        "max_grid_size_z" : max_grid_size_z,
        "min_grid_size_x" : min_grid_size_x,
        "min_grid_size_y" : min_grid_size_y,
        "min_grid_size_z" : min_grid_size_z,
        "stencil_d_max" : stencil_d_max,
        "valid_tile_sizes" : valid_tile_sizes
    }

class SilentUndefined(Undefined):
    """Custom Undefined class that returns sensible defaults for missing variables."""
    
    def __int__(self):
        return 0
    
    def __float__(self):
        return 0.0
    
    def __str__(self):
        return ""
    
    def __iter__(self):
        return iter([])
    
    def __len__(self):
        return 0
    
    def __bool__(self):
        return False
    
    def __getattr__(self, name):
        return SilentUndefined()

def render_template(template_path: str, design_parameters: dict):
    """Render a Jinja2 template with design parameters.
    
    Args:
        template_path: Path to the .j2 template file
        design_parameters: Dict of parameters to pass to the template
    
    Returns:
        Tuple of (output_filename, rendered_content)
    """
    # Get directory and filename
    template_dir = os.path.dirname(template_path)
    template_filename = os.path.basename(template_path)
    
    # Extract output filename by removing .j2 extension
    if template_filename.endswith('.j2'):
        output_filename = template_filename[:-3]
    else:
        output_filename = template_filename + ".out"
    
    output_path = os.path.join(template_dir, output_filename)
    
    # Create Jinja2 environment
    env = Environment(
        loader=FileSystemLoader(template_dir if template_dir else '.')
       #,undefined=SilentUndefined
    )
    
    # Load and render template
    template = env.get_template(template_filename)
    rendered_content = template.render(**design_parameters)
    
    return output_path, rendered_content


def generate_files(design_parameters: dict):
    """Generate top.cpp and top.hpp files from templates using design parameters.
    
    Args:
        design_parameters: Dict of design parameters to pass to templates
    """
    templates = [top_cpp_template, top_hpp_template, testbench_cpp_template]
    
    for template_path in templates:
        output_path, rendered_content = render_template(template_path, design_parameters)
        
        # Write rendered content to output file
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(rendered_content)
        
        print(f"Generated: {output_path}")
    
    # Create .generated marker file to indicate successful generation
    generated_marker = os.path.join(os.path.dirname(__file__), ".generated")
    with open(generated_marker, 'w', encoding='utf-8') as f:
        f.write("")
    
    print(f"Generated: {generated_marker}")
    


def main():
    parser = argparse.ArgumentParser(description="Generate design parameters from constraints")
    parser.add_argument("-rs", "--random_seed", type=int, default=None,
                        help="Random seed for reproducibility (optional)")
    args = parser.parse_args()
    
    data = read_constrains()
    # minimal behavior: print the dict
    print("\n===============")
    print("JSON_CONTRAINTS")
    print("---------------")
    
    for record in data:
        print(f"{record}: {data[record]}")
    print("===============\n")

    design_paramters = generate_design_parameter(data, random_seed=args.random_seed)
    
    print("===============")
    print("DESIGN_PARAMETERS")
    print("---------------")
    
    for record in design_paramters:
        print(f"{record}: {design_paramters[record]}")
    print("===============\n")
    
    # Generate top.cpp, main.cpp and top.hpp files
    generate_files(design_paramters)


if __name__ == "__main__":
    main()