"""Tạo report/template.docx (reference doc cho pandoc) từ reference.docx mặc định của pandoc.

Định dạng: A4, lề trái 3 cm, phải/trên/dưới 2 cm; Times New Roman 13; giãn dòng 1,5; đoạn văn căn đều,
thụt đầu dòng 1 cm; chương (Heading 1) sang trang mới, chữ in hoa, căn giữa; caption hình/bảng căn giữa;
bảng có kẻ ô; số trang ở giữa chân trang; Word được nhắc cập nhật field (mục lục) khi mở file.
Đây là mẫu tạm, sẽ thay bằng mẫu của khoa (build_report.sh dùng bất kỳ file nào ở report/template.docx).

Dùng: .venv/bin/python scripts/make_template.py [--out report/template.docx]
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import zipfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent

TNR = ('<w:rFonts w:ascii="Times New Roman" w:hAnsi="Times New Roman" '
       'w:eastAsia="Times New Roman" w:cs="Times New Roman"/>')
MONO = '<w:rFonts w:ascii="Consolas" w:hAnsi="Consolas" w:cs="Consolas"/>'

DOC_DEFAULTS = f"""<w:docDefaults>
    <w:rPrDefault><w:rPr>{TNR}<w:sz w:val="26"/><w:szCs w:val="26"/>
      <w:lang w:val="vi-VN" w:eastAsia="en-US" w:bidi="ar-SA"/></w:rPr></w:rPrDefault>
    <w:pPrDefault><w:pPr><w:spacing w:after="120" w:line="360" w:lineRule="auto"/></w:pPr></w:pPrDefault>
  </w:docDefaults>"""


def para(style_id: str, name: str, ppr: str = "", rpr: str = "", based: str = "Normal",
         extra: str = "", custom: bool = False) -> str:
    c = ' w:customStyle="1"' if custom else ""
    b = f'<w:basedOn w:val="{based}"/>' if based else ""
    return (f'<w:style w:type="paragraph"{c} w:styleId="{style_id}"><w:name w:val="{name}"/>{b}{extra}'
            f'<w:qFormat/><w:pPr>{ppr}</w:pPr><w:rPr>{rpr}</w:rPr></w:style>')


# Các style được thay toàn bộ (theo styleId). Style không có ở đây giữ nguyên như mặc định của pandoc.
STYLES = {
    "Normal": '<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/>'
              '<w:qFormat/></w:style>',
    "BodyText": para("BodyText", "Body Text",
                     '<w:spacing w:before="0" w:after="120"/><w:ind w:firstLine="567"/><w:jc w:val="both"/>',
                     extra='<w:link w:val="BodyTextChar"/>'),
    "FirstParagraph": para("FirstParagraph", "First Paragraph", based="BodyText",
                           extra='<w:next w:val="BodyText"/>', custom=True),
    # Ô bảng: không thụt dòng, giãn dòng 1,15, cỡ 12.
    "Compact": para("Compact", "Compact",
                    '<w:spacing w:before="36" w:after="36" w:line="276" w:lineRule="auto"/><w:jc w:val="left"/>',
                    '<w:sz w:val="24"/><w:szCs w:val="24"/>', custom=True),
    # Mục danh sách (gạch đầu dòng, đánh số, mã giả): như thân bài, cỡ 13, giãn dòng 1,5, căn đều.
    # scripts/fix_docx.py chuyển các đoạn Compact có đánh số sang style này sau pandoc.
    "ListCompact": para("ListCompact", "List Compact",
                        '<w:spacing w:before="0" w:after="60" w:line="360" w:lineRule="auto"/><w:jc w:val="both"/>',
                        custom=True),
    "Title": para("Title", "Title", '<w:keepNext/><w:spacing w:before="240" w:after="240"/><w:jc w:val="center"/>',
                  '<w:b/><w:bCs/><w:sz w:val="32"/><w:szCs w:val="32"/>', extra='<w:next w:val="BodyText"/>'),
    "Subtitle": para("Subtitle", "Subtitle", '<w:spacing w:before="120" w:after="120"/><w:jc w:val="center"/>',
                     '<w:sz w:val="28"/><w:szCs w:val="28"/>', based="Title"),
    "Heading1": para("Heading1", "Heading 1",
                     '<w:keepNext/><w:keepLines/><w:pageBreakBefore/><w:spacing w:before="0" w:after="240"/>'
                     '<w:jc w:val="center"/><w:outlineLvl w:val="0"/>',
                     '<w:b/><w:bCs/><w:caps/><w:sz w:val="32"/><w:szCs w:val="32"/>',
                     extra='<w:next w:val="BodyText"/><w:uiPriority w:val="9"/>'),
    "Heading2": para("Heading2", "Heading 2",
                     '<w:keepNext/><w:keepLines/><w:spacing w:before="240" w:after="120"/><w:outlineLvl w:val="1"/>',
                     '<w:b/><w:bCs/><w:sz w:val="28"/><w:szCs w:val="28"/>',
                     extra='<w:next w:val="BodyText"/><w:uiPriority w:val="9"/>'),
    "Heading3": para("Heading3", "Heading 3",
                     '<w:keepNext/><w:keepLines/><w:spacing w:before="180" w:after="120"/><w:outlineLvl w:val="2"/>',
                     '<w:b/><w:bCs/><w:i/><w:iCs/><w:sz w:val="26"/><w:szCs w:val="26"/>',
                     extra='<w:next w:val="BodyText"/><w:uiPriority w:val="9"/>'),
    "Caption": para("Caption", "Caption", '<w:spacing w:before="60" w:after="200" w:line="276" w:lineRule="auto"/>'
                    '<w:jc w:val="center"/>', '<w:i/><w:iCs/><w:sz w:val="24"/><w:szCs w:val="24"/>'),
    "TableCaption": para("TableCaption", "Table Caption", '<w:keepNext/><w:spacing w:before="200" w:after="60"/>',
                         based="Caption", custom=True),
    "ImageCaption": para("ImageCaption", "Image Caption", based="Caption", custom=True),
    "Figure": para("Figure", "Figure", '<w:spacing w:before="120" w:after="0"/><w:jc w:val="center"/>',
                   custom=True),
    "CaptionedFigure": para("CaptionedFigure", "Captioned Figure", '<w:keepNext/><w:jc w:val="center"/>',
                            based="Figure", custom=True),
    "TOCHeading": para("TOCHeading", "TOC Heading", '<w:outlineLvl w:val="9"/>', based="Heading1",
                       extra='<w:next w:val="BodyText"/><w:uiPriority w:val="39"/>'),
    "Bibliography": para("Bibliography", "Bibliography",
                         '<w:spacing w:after="80"/><w:ind w:left="567" w:hanging="567"/><w:jc w:val="both"/>',
                         extra='<w:next w:val="Bibliography"/>'),
    "VerbatimChar": '<w:style w:type="character" w:customStyle="1" w:styleId="VerbatimChar">'
                    '<w:name w:val="Verbatim Char"/><w:basedOn w:val="BodyTextChar"/>'
                    f'<w:rPr>{MONO}<w:sz w:val="22"/><w:szCs w:val="22"/></w:rPr></w:style>',
    # Khối mã (pandoc dùng style "Source Code" nếu reference doc có).
    "SourceCode": para("SourceCode", "Source Code",
                       '<w:wordWrap w:val="off"/><w:shd w:val="clear" w:color="auto" w:fill="F2F2F2"/>'
                       '<w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/><w:jc w:val="left"/>',
                       f'{MONO}<w:sz w:val="19"/><w:szCs w:val="19"/>', extra='<w:link w:val="VerbatimChar"/>',
                       custom=True),
    # Style riêng cho bìa và các danh mục (dùng qua ::: {custom-style="..."}).
    "CoverText": para("CoverText", "Cover Text", '<w:spacing w:before="0" w:after="60"/><w:jc w:val="center"/>',
                      '<w:b/><w:bCs/><w:sz w:val="26"/><w:szCs w:val="26"/>', custom=True),
    "CoverTitle": para("CoverTitle", "Cover Title", '<w:spacing w:before="480" w:after="480"/><w:jc w:val="center"/>',
                       '<w:b/><w:bCs/><w:sz w:val="32"/><w:szCs w:val="32"/>', custom=True),
    "CoverInfo": para("CoverInfo", "Cover Info", '<w:spacing w:before="0" w:after="60"/><w:ind w:left="2268"/>',
                      custom=True),
    "ListEntry": para("ListEntry", "List Entry", '<w:spacing w:before="0" w:after="60"/><w:ind w:left="1134" '
                      'w:hanging="1134"/><w:jc w:val="left"/>', custom=True),
}

TABLE_STYLE = """<w:style w:type="table" w:default="1" w:styleId="Table">
    <w:name w:val="Table"/><w:basedOn w:val="TableNormal"/><w:qFormat/>
    <w:tblPr><w:jc w:val="center"/><w:tblInd w:w="0" w:type="dxa"/>
      <w:tblBorders>
        <w:top w:val="single" w:sz="4" w:space="0" w:color="000000"/>
        <w:left w:val="single" w:sz="4" w:space="0" w:color="000000"/>
        <w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/>
        <w:right w:val="single" w:sz="4" w:space="0" w:color="000000"/>
        <w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/>
        <w:insideV w:val="single" w:sz="4" w:space="0" w:color="000000"/>
      </w:tblBorders>
      <w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:left w:w="85" w:type="dxa"/>
        <w:bottom w:w="0" w:type="dxa"/><w:right w:w="85" w:type="dxa"/></w:tblCellMar>
    </w:tblPr>
    <w:tblStylePr w:type="firstRow"><w:rPr><w:b/><w:bCs/></w:rPr>
      <w:tcPr><w:shd w:val="clear" w:color="auto" w:fill="E7E6E6"/></w:tcPr></w:tblStylePr>
  </w:style>"""

FOOTER = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" \
xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<w:p><w:pPr><w:jc w:val="center"/></w:pPr>
<w:r><w:fldChar w:fldCharType="begin"/></w:r><w:r><w:instrText xml:space="preserve"> PAGE </w:instrText></w:r>
<w:r><w:fldChar w:fldCharType="separate"/></w:r><w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType="end"/></w:r>
</w:p></w:ftr>"""

# A4; lề tính bằng twip (1 cm = 567): trên 2, phải 2, dưới 2, trái 3.
SECT_PR = ('<w:sectPr><w:footerReference w:type="default" r:id="rIdNocFooter"/>'
           '<w:pgSz w:w="11906" w:h="16838"/>'
           '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1701" w:header="567" w:footer="567" '
           'w:gutter="0"/></w:sectPr>')


def replace_style(xml: str, style_id: str, new: str) -> str:
    pat = re.compile(r'<w:style\b[^>]*w:styleId="%s"[^>]*>.*?</w:style>' % re.escape(style_id), re.S)
    if pat.search(xml):
        return pat.sub(lambda _: new, xml, count=1)
    return xml.replace("</w:styles>", new + "\n</w:styles>")


def fix_styles(xml: str) -> str:
    xml = re.sub(r"<w:docDefaults>.*?</w:docDefaults>", lambda _: DOC_DEFAULTS, xml, count=1, flags=re.S)
    for sid, new in STYLES.items():
        xml = replace_style(xml, sid, new)
    xml = replace_style(xml, "Table", TABLE_STYLE)
    # Các style còn lại (Heading 4–9, Abstract…): bỏ font theme và màu xanh.
    xml = re.sub(r"<w:rFonts [^>]*Theme[^>]*/>", TNR, xml)
    xml = re.sub(r"<w:color [^>]*/>", "", xml)
    xml = xml.replace("/>>", "/>")  # lỗi gõ có sẵn trong style AbstractTitle của pandoc
    return xml


def build(out: Path) -> None:
    ref = subprocess.run([os.environ.get("PANDOC", "pandoc"), "--print-default-data-file", "reference.docx"],
                         capture_output=True, check=True).stdout
    tmp = out.with_suffix(".pandoc-default.docx")
    tmp.write_bytes(ref)
    with zipfile.ZipFile(tmp) as zin, zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "word/styles.xml":
                data = fix_styles(data.decode("utf-8")).encode("utf-8")
            elif item.filename == "word/document.xml":
                s = data.decode("utf-8")
                s, n = re.subn(r"<w:sectPr\s*/>|<w:sectPr>.*?</w:sectPr>", SECT_PR, s, flags=re.S)
                assert n == 1, "không tìm thấy sectPr trong document.xml"
                data = s.encode("utf-8")
            elif item.filename == "word/settings.xml":
                s = data.decode("utf-8")
                s = s.replace("<w:zoom ", '<w:updateFields w:val="true"/><w:zoom ', 1)
                s = s.replace('<w:themeFontLang w:val="en-US" />', '<w:themeFontLang w:val="vi-VN" />')
                data = s.encode("utf-8")
            elif item.filename == "word/_rels/document.xml.rels":
                data = data.decode("utf-8").replace(
                    "</Relationships>",
                    '<Relationship Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/'
                    'footer" Id="rIdNocFooter" Target="footer1.xml" /></Relationships>').encode("utf-8")
            elif item.filename == "[Content_Types].xml":
                data = data.decode("utf-8").replace(
                    "</Types>",
                    '<Override PartName="/word/footer1.xml" ContentType="application/'
                    'vnd.openxmlformats-officedocument.wordprocessingml.footer+xml" /></Types>').encode("utf-8")
            # Ngày cố định: chạy lại cho ra cùng một file.
            zout.writestr(zipfile.ZipInfo(item.filename, date_time=(2026, 1, 1, 0, 0, 0)), data,
                          compress_type=zipfile.ZIP_DEFLATED)
        zout.writestr(zipfile.ZipInfo("word/footer1.xml", date_time=(2026, 1, 1, 0, 0, 0)), FOOTER,
                      compress_type=zipfile.ZIP_DEFLATED)
    tmp.unlink()


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=str(REPO / "report" / "template.docx"))
    args = ap.parse_args()
    build(Path(args.out))
    print(f"đã ghi {args.out}")


if __name__ == "__main__":
    main()
