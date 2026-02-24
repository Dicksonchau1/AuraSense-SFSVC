#!/usr/bin/env python3
"""Generate professional PDFs from AISS application markdown files."""

import markdown
from weasyprint import HTML
from pathlib import Path

# Professional CSS for PDF output
CSS = """
@page {
    size: A4;
    margin: 2.2cm 2cm;
    @bottom-center {
        content: "AuraSense Limited — AISS Application 2026";
        font-size: 8pt;
        color: #94a3b8;
        font-family: 'Segoe UI', Arial, sans-serif;
    }
    @bottom-right {
        content: "Page " counter(page) " of " counter(pages);
        font-size: 8pt;
        color: #94a3b8;
        font-family: 'Segoe UI', Arial, sans-serif;
    }
}

body {
    font-family: 'Segoe UI', 'Helvetica Neue', Arial, sans-serif;
    font-size: 10.5pt;
    line-height: 1.65;
    color: #1e293b;
    max-width: 100%;
}

h1 {
    font-size: 20pt;
    font-weight: 800;
    color: #0f172a;
    border-bottom: 3px solid #3b82f6;
    padding-bottom: 8px;
    margin-top: 0;
    margin-bottom: 16px;
}

h2 {
    font-size: 15pt;
    font-weight: 700;
    color: #1e40af;
    margin-top: 28px;
    margin-bottom: 10px;
    border-bottom: 1px solid #e2e8f0;
    padding-bottom: 5px;
}

h3 {
    font-size: 12pt;
    font-weight: 700;
    color: #1e293b;
    margin-top: 20px;
    margin-bottom: 8px;
}

h4 {
    font-size: 10.5pt;
    font-weight: 700;
    color: #334155;
    margin-top: 16px;
    margin-bottom: 6px;
}

p {
    margin-bottom: 8px;
}

strong {
    color: #0f172a;
}

table {
    width: 100%;
    border-collapse: collapse;
    margin: 12px 0 18px 0;
    font-size: 9.5pt;
}

th, td {
    padding: 8px 10px;
    text-align: left;
    border: 1px solid #cbd5e1;
    vertical-align: top;
}

th {
    background: #f1f5f9;
    font-weight: 700;
    color: #1e293b;
    font-size: 9pt;
    text-transform: uppercase;
    letter-spacing: 0.03em;
}

td {
    background: #ffffff;
}

tr:nth-child(even) td {
    background: #f8fafc;
}

blockquote {
    border-left: 4px solid #3b82f6;
    margin: 12px 0;
    padding: 8px 16px;
    background: #eff6ff;
    color: #1e40af;
    font-size: 9.5pt;
    border-radius: 0 6px 6px 0;
}

code {
    font-family: 'Cascadia Code', 'JetBrains Mono', 'Consolas', monospace;
    background: #f1f5f9;
    padding: 1px 5px;
    border-radius: 3px;
    font-size: 9pt;
    color: #334155;
}

pre {
    background: #0f172a;
    color: #e2e8f0;
    padding: 14px 18px;
    border-radius: 8px;
    font-size: 8.5pt;
    line-height: 1.5;
    overflow-x: auto;
    margin: 12px 0;
}

pre code {
    background: none;
    padding: 0;
    color: inherit;
    font-size: inherit;
}

ul, ol {
    margin: 8px 0;
    padding-left: 22px;
}

li {
    margin-bottom: 4px;
}

hr {
    border: none;
    border-top: 1px solid #e2e8f0;
    margin: 20px 0;
}

em {
    color: #64748b;
}

/* Checkbox styling */
input[type="checkbox"] {
    margin-right: 6px;
}

/* Special highlight for SFSVC mentions */
a {
    color: #2563eb;
    text-decoration: none;
}
"""

# Cover page CSS (first doc only)
COVER_CSS = CSS + """
.cover {
    text-align: center;
    padding-top: 120px;
}
.cover h1 {
    font-size: 28pt;
    border: none;
    color: #1e40af;
    margin-bottom: 12px;
}
.cover .subtitle {
    font-size: 16pt;
    color: #475569;
    font-weight: 400;
    margin-bottom: 40px;
}
.cover .company {
    font-size: 14pt;
    font-weight: 700;
    color: #0f172a;
    margin-bottom: 6px;
}
.cover .date {
    font-size: 11pt;
    color: #64748b;
}
.cover .divider {
    width: 80px;
    height: 4px;
    background: #3b82f6;
    margin: 30px auto;
    border-radius: 2px;
}
"""


def md_to_pdf(md_path: Path, pdf_path: Path, css: str = CSS):
    """Convert a markdown file to a styled PDF."""
    md_text = md_path.read_text(encoding="utf-8")
    
    # Convert markdown to HTML
    html_body = markdown.markdown(
        md_text,
        extensions=["tables", "fenced_code", "nl2br", "sane_lists"],
    )
    
    full_html = f"""<!DOCTYPE html>
<html lang="en">
<head><meta charset="UTF-8"><style>{css}</style></head>
<body>{html_body}</body>
</html>"""
    
    HTML(string=full_html).write_pdf(str(pdf_path))
    size_kb = pdf_path.stat().st_size / 1024
    print(f"  ✓ {pdf_path.name} ({size_kb:.0f} KB)")


def generate_cover_page(pdf_path: Path):
    """Generate a cover page PDF."""
    cover_html = f"""<!DOCTYPE html>
<html lang="en">
<head><meta charset="UTF-8"><style>{COVER_CSS}</style></head>
<body>
<div class="cover">
    <h1>Artificial Intelligence<br>Subsidy Scheme (AISS)</h1>
    <p class="subtitle">Application Package</p>
    <div class="divider"></div>
    <p class="company">AuraSense Limited<br>歐雅生物科技有限公司</p>
    <p class="date">February 2026</p>
    <br><br><br>
    <p style="font-size: 11pt; color: #475569;">
        <strong>Project:</strong> Neuromorphic SFSVC Engine for<br>
        Real-Time Drone Crack Inspection
    </p>
    <br>
    <p style="font-size: 9pt; color: #94a3b8;">
        Submitted to Hong Kong Cyberport Management Company Limited (HKCMCL)<br>
        under the Artificial Intelligence Subsidy Scheme
    </p>
</div>
</body>
</html>"""
    
    HTML(string=cover_html).write_pdf(str(pdf_path))
    size_kb = pdf_path.stat().st_size / 1024
    print(f"  ✓ {pdf_path.name} ({size_kb:.0f} KB)")


def main():
    app_dir = Path(__file__).parent
    pdf_dir = app_dir / "pdf"
    pdf_dir.mkdir(exist_ok=True)
    
    print("=" * 55)
    print("  AuraSense AISS Application — PDF Generation")
    print("=" * 55)
    print()
    
    # 1. Cover page
    print("Generating cover page...")
    generate_cover_page(pdf_dir / "00_Cover_Page.pdf")
    
    # 2. Application documents
    docs = [
        ("AISS_Application_Form_AuraSense.md", "01_AISS_Application_Form.pdf"),
        ("Appendix_A_Project_Proposal.md", "02_Appendix_A_Project_Proposal.pdf"),
        ("Appendix_B_Team_Structure_CV.md", "03_Appendix_B_Team_Structure_CV.pdf"),
        ("Appendix_C_Track_Records.md", "04_Appendix_C_Track_Records.pdf"),
        ("SUBMISSION_CHECKLIST.md", "05_Submission_Checklist.pdf"),
    ]
    
    print("\nGenerating application documents...")
    for md_name, pdf_name in docs:
        md_path = app_dir / md_name
        if md_path.exists():
            md_to_pdf(md_path, pdf_dir / pdf_name)
        else:
            print(f"  ✗ {md_name} not found — skipped")
    
    print("\n" + "=" * 55)
    print(f"  All PDFs saved to: {pdf_dir}")
    print("=" * 55)
    
    # List final files
    print("\nFinal package:")
    total = 0
    for f in sorted(pdf_dir.glob("*.pdf")):
        size = f.stat().st_size / 1024
        total += size
        print(f"  📄 {f.name:45s} {size:6.0f} KB")
    print(f"  {'─' * 55}")
    print(f"  {'Total':45s} {total:6.0f} KB")


if __name__ == "__main__":
    main()
