#!/usr/bin/env python3
# /// script
# requires-python = ">=3.11"
# dependencies = [
#   "anthropic>=0.40.0",
#   "mistralai>=2.0.0",
# ]
# ///
# =============================================================================
# Bitcrush Testing 2026
# =============================================================================

"""
generate_blackbox_tests.py

Workflow:
  1. Run Doxygen on the target header to produce an XML description
  2. Parse the XML to extract the documented public API contract as plain text
  3. Send the contract to the chosen LLM provider with a black-box test prompt
  4. Write the generated GTest file to disk

The implementation file (.cpp) is never read or sent to the API.
Only the Doxygen-extracted documentation is transmitted.

Usage:
    python generate_blackbox_tests.py [--header PATH] [--output PATH]
                                      [--provider anthropic|mistral]
                                      [--model MODEL]

Requirements:
    pip install anthropic mistralai
    doxygen must be on PATH

Environment:
    ANTHROPIC_API_KEY   -- required when --provider anthropic (default)
    MISTRAL_API_KEY     -- required when --provider mistral
"""

import argparse
import os
import subprocess
import sys
import tempfile
import textwrap
import xml.etree.ElementTree as ET
from pathlib import Path

# ── Defaults ──────────────────────────────────────────────────────────────────

DEFAULT_HEADER   = "pressure_regulator.h"
DEFAULT_OUTPUT   = "generated_tests/blackbox_tests.cpp"
DEFAULT_PROVIDER = "anthropic"

PROVIDER_DEFAULTS = {
    "anthropic": "claude-sonnet-4-6",
    "mistral":   "mistral-large-latest",
}

# ── Doxygen configuration ─────────────────────────────────────────────────────

DOXYFILE_TEMPLATE = """
PROJECT_NAME           = BlackBoxInput
INPUT                  = {header_path}
GENERATE_XML           = YES
GENERATE_HTML          = NO
GENERATE_LATEX         = NO
OUTPUT_DIRECTORY       = {output_dir}
EXTRACT_ALL            = NO
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = NO
QUIET                  = YES
WARNINGS               = NO
XML_PROGRAMLISTING     = NO
SHOW_USED_FILES        = NO
"""

# ── Step 1: Run Doxygen ───────────────────────────────────────────────────────

def run_doxygen(header_path: Path, work_dir: Path) -> Path:
    """Run Doxygen on the header and return the path to the XML output directory."""
    doxyfile = work_dir / "Doxyfile"
    xml_dir  = work_dir / "xml"

    doxyfile.write_text(
        DOXYFILE_TEMPLATE.format(
            header_path=header_path.resolve(),
            output_dir=work_dir.resolve(),
        )
    )

    result = subprocess.run(
        ["doxygen", str(doxyfile)],
        capture_output=True,
        text=True,
    )

    if result.returncode != 0:
        print("Doxygen failed:", result.stderr, file=sys.stderr)
        sys.exit(1)

    if not xml_dir.exists():
        print("Doxygen produced no XML output.", file=sys.stderr)
        sys.exit(1)

    return xml_dir

# ── Step 2: Parse Doxygen XML into plain text ─────────────────────────────────

def text_of(element) -> str:
    """Recursively extract all text from a Doxygen XML element."""
    parts = []
    if element.text:
        parts.append(element.text.strip())
    for child in element:
        parts.append(text_of(child))
        if child.tail:
            parts.append(child.tail.strip())
    return " ".join(p for p in parts if p)


def parse_doxygen_xml(xml_dir: Path) -> str:
    """
    Walk the Doxygen XML and extract a plain-text contract description.
    Returns a formatted string suitable for inclusion in a prompt.
    """
    index_file = xml_dir / "index.xml"
    if not index_file.exists():
        print("index.xml not found in Doxygen output.", file=sys.stderr)
        sys.exit(1)

    index    = ET.parse(index_file)
    sections = []

    for compound in index.getroot().findall("compound"):
        kind    = compound.get("kind")
        refid   = compound.get("refid")
        name_el = compound.find("name")
        name    = name_el.text if name_el is not None else "(unknown)"

        xml_file = xml_dir / f"{refid}.xml"
        if not xml_file.exists():
            continue

        tree = ET.parse(xml_file)
        root = tree.getroot()

        for compounddef in root.findall("compounddef"):
            brief  = compounddef.find("briefdescription")
            detail = compounddef.find("detaileddescription")

            block = [f"\n{'='*60}", f"{kind.upper()}: {name}", "="*60]

            if brief is not None:
                t = text_of(brief)
                if t:
                    block.append(f"Brief: {t}")

            if detail is not None:
                t = text_of(detail)
                if t:
                    block.append(f"Description:\n{textwrap.fill(t, width=80)}")

            for member in compounddef.iter("memberdef"):
                m_kind   = member.get("kind", "")
                m_name   = text_of(member.find("name"))              if member.find("name")              is not None else ""
                m_brief  = text_of(member.find("briefdescription"))  if member.find("briefdescription")  is not None else ""
                m_detail = text_of(member.find("detaileddescription")) if member.find("detaileddescription") is not None else ""
                m_type   = text_of(member.find("type"))              if member.find("type")              is not None else ""

                entry = [f"\n  {m_kind.upper()}: {m_name}"]
                if m_type:
                    entry.append(f"    Type/Returns: {m_type}")
                if m_brief:
                    entry.append(f"    Brief: {m_brief}")
                if m_detail:
                    entry.append(
                        f"    Detail: {textwrap.fill(m_detail, width=76, initial_indent='    ', subsequent_indent='    ')}"
                    )

                for param in member.findall(".//param"):
                    pname = text_of(param.find("declname")) if param.find("declname") is not None else ""
                    ptype = text_of(param.find("type"))     if param.find("type")     is not None else ""
                    if pname:
                        entry.append(f"    Param: {ptype} {pname}")

                block.extend(entry)

            sections.append("\n".join(block))

    return "\n".join(sections)

# ── Step 3: Prompts ───────────────────────────────────────────────────────────

SYSTEM_PROMPT = """\
You are an expert C++ software engineer specialising in unit testing for
embedded Linux systems. You write GTest/GMock unit tests using a strict
black-box approach: you work only from the documented public API contract
provided to you. You never invent implementation details, never access
private members, and never assume behaviour that is not explicitly documented.

Rules:
- Use GTest (TEST_F, EXPECT_*, ASSERT_*) and GMock (HasSubstr) only.
- Define all required test doubles (stubs, spies) inside the test file.
- Each test covers exactly one documented clause.
- Add a short comment above each test citing the documentation clause it verifies.
- Use a test fixture that creates fresh objects for every test.
- Do not include any #include for the implementation file.
- Output only the complete C++ source file. No explanation, no markdown fences.
"""

def build_user_message(header_name: str, contract: str) -> str:
    return (
        f'The following is the public API contract extracted from the Doxygen '
        f'documentation of "{header_name}". '
        f'The implementation file was NOT provided and must NOT be assumed.\n\n'
        f'Write a complete GTest black-box unit test file covering every '
        f'documented state, transition, postcondition, boundary value, and '
        f'fault condition described below.\n\n'
        f'--- CONTRACT START ---\n{contract}\n--- CONTRACT END ---'
    )

# ── Step 4a: Anthropic ────────────────────────────────────────────────────────

def call_anthropic(contract: str, header_name: str, model: str) -> str:
    import anthropic

    api_key = os.environ.get("ANTHROPIC_API_KEY")
    if not api_key:
        print("ANTHROPIC_API_KEY environment variable is not set.", file=sys.stderr)
        sys.exit(1)

    print(f"Sending contract to Anthropic ({model})...")

    client  = anthropic.Anthropic(api_key=api_key, timeout=120.0)  # 120 second timeout
    message = client.messages.create(
        model=model,
        max_tokens=4096,
        system=SYSTEM_PROMPT,
        messages=[{"role": "user", "content": build_user_message(header_name, contract)}],
    )
    return message.content[0].text

# ── Step 4b: Mistral ──────────────────────────────────────────────────────────

def call_mistral(contract: str, header_name: str, model: str) -> str:
    from mistralai.client import Mistral  # v2+ import path

    api_key = os.environ.get("MISTRAL_API_KEY")
    if not api_key:
        print("MISTRAL_API_KEY environment variable is not set.", file=sys.stderr)
        sys.exit(1)

    print(f"Sending contract to Mistral ({model})...")

    client   = Mistral(api_key=api_key, timeout_ms=120_000)  # 120 second timeout
    response = client.chat.complete(
        model=model,
        messages=[
            {"role": "system",  "content": SYSTEM_PROMPT},
            {"role": "user",    "content": build_user_message(header_name, contract)},
        ],
    )
    return response.choices[0].message.content

# ── Step 4: Dispatch ──────────────────────────────────────────────────────────

def generate_tests(provider: str, contract: str, header_name: str, model: str) -> str:
    if provider == "anthropic":
        return call_anthropic(contract, header_name, model)
    elif provider == "mistral":
        return call_mistral(contract, header_name, model)
    else:
        print(f"Unknown provider: {provider}", file=sys.stderr)
        sys.exit(1)

# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Generate black-box GTest unit tests from Doxygen documentation."
    )
    parser.add_argument(
        "--header",
        default=DEFAULT_HEADER,
        help=f"Path to the C++ header file (default: {DEFAULT_HEADER})",
    )
    parser.add_argument(
        "--output",
        default=DEFAULT_OUTPUT,
        help=f"Path to write the generated test file (default: {DEFAULT_OUTPUT})",
    )
    parser.add_argument(
        "--provider",
        default=DEFAULT_PROVIDER,
        choices=["anthropic", "mistral"],
        help=f"LLM provider to use (default: {DEFAULT_PROVIDER})",
    )
    parser.add_argument(
        "--model",
        default=None,
        help=(
            "Model name to use. "
            f"Defaults: anthropic={PROVIDER_DEFAULTS['anthropic']}, "
            f"mistral={PROVIDER_DEFAULTS['mistral']}"
        ),
    )
    args = parser.parse_args()

    header_path = Path(args.header)
    output_path = Path(args.output)
    model       = args.model or PROVIDER_DEFAULTS[args.provider]

    # Preflight checks
    if subprocess.run(["which", "doxygen"], capture_output=True).returncode != 0:
        print(
            "doxygen not found.\n"
            "Install with:  brew install doxygen      (macOS)\n"
            "               apt install doxygen       (Linux)",
            file=sys.stderr,
        )
        sys.exit(1)

    if not header_path.exists():
        print(f"Header not found: {header_path}", file=sys.stderr)
        sys.exit(1)

    with tempfile.TemporaryDirectory() as tmp:
        work_dir = Path(tmp)

        # Step 1: Doxygen
        print(f"Running Doxygen on {header_path.name}...")
        xml_dir = run_doxygen(header_path, work_dir)

        # Step 2: Parse XML into plain-text contract
        print("Parsing Doxygen XML...")
        contract = parse_doxygen_xml(xml_dir)

        if not contract.strip():
            print("No documentation found in Doxygen output.", file=sys.stderr)
            sys.exit(1)

    # Step 3+4: Call provider
    generated = generate_tests(args.provider, contract, header_path.name, model)

    # Step 5: Write output (create directory if it does not exist)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(generated)
    print(f"Tests written to: {output_path}")


if __name__ == "__main__":
    main()