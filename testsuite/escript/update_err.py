import sys
from pathlib import Path
import subprocess

# Too lazy to make recursion + error checking in Python... But watch out for
# `escript/in/inc0{1,2}.err` because they give different error messages across
# POSIX and Windows.
#
#   find . -iname '*.err' -exec python3 update_err.py {} \;

def read_file(file_path):
  try:
    with open(file_path, 'r') as file:
      file_contents = file.read()
      return file_contents.strip()
  except FileNotFoundError:
    print(f"File '{file_path}' not found.")
    return None

if __name__ == "__main__":
  if len(sys.argv) < 2:
    print(f"Usage: python {sys.argv[0]} <file_path>")
  else:
    file_path = Path(sys.argv[1])

    if file_path.is_file():
      file_src = file_path.with_suffix(".src")
      file_err = file_path.with_suffix(".err")

      contents = read_file(file_err)
      if contents is None:
        sys.exit(1)

      index = contents.find(": error: ")
      if index != -1:
        contents = contents[index + len(": error: "):]

      relative_src = file_src.relative_to(Path.cwd()) if file_src.is_absolute() else file_src

      # Run the "ecompile" program with file_src as the first argument
      result = subprocess.run(["../../bin/ecompile", "-C", "ecompile.cfg", str(relative_src)], capture_output=True, text=True)

      # Store both stderr and stdout into the same variable
      output = result.stderr

      # Find the line that contains 'contents'
      lines = output.split('\n')
      line_with_contents = next((line for line in lines if contents in line), None)

      if line_with_contents:
        # :line:column: error message
        # err_string = f"{file_src.name}{line_with_contents[line_with_contents.find(":"):]}"

        # :line:column: error message
        err_string = line_with_contents[line_with_contents.find(":"):]
        print(err_string)
        with open(file_err, 'w') as file:
          file.write(err_string)
      else:
        print(f"No line containing\n-\n{contents}\n-\nfound in\n-\n{output}\n-")
    else:
      print(f"File '{file_path}' not found.")
