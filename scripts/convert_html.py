Import("env")
from pathlib import Path
import re

# src/ Ordner über PlatformIO-Variable
src_dir = Path(env.subst("$PROJECT_SRC_DIR"))
out_dir = src_dir / "generated" / "html"
out_dir.mkdir(parents=True, exist_ok=True)

web_dir = src_dir / "web" / "server" / "html"

def minify_html(html: str) -> str:
    import re
    html = re.sub(r'<!--.*?-->', '', html, flags=re.DOTALL)
    html = re.sub(r'\s+', ' ', html)
    html = re.sub(r'>\s+<', '><', html)
    return html.strip()

def escape_cpp_string(text: str) -> str:
    return (
        text.replace("\\", "\\\\")
            .replace("\"", "\\\"")
            .replace("\n", "\\n\"\n\"")
    )

for file in web_dir.rglob("*.html"):
    content = file.read_text(encoding="utf-8")
    content = minify_html(content)
    escaped = escape_cpp_string(content)

    var_name = file.stem
    cpp_path = out_dir / f"{var_name}.cpp"

    cpp = (
        '#include "html.hpp"\n\n'
        "namespace Html {\n"
        f"    std::string {var_name} = \"{escaped}\";\n"
        "}\n"
    )

    cpp_path.write_text(cpp, encoding="utf-8")
    print("Generated:", cpp_path)
