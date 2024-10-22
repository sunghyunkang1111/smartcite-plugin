import sys
import requests
import os
import subprocess
from urllib.parse import unquote

from pypdf import PdfReader




def download_pdf(pdf_url, local_file):
    # pdf_url = unquote(pdf_url)
    # Download the PDF from the URL
    response = requests.get(pdf_url)
    
    if response.status_code == 200:
        with open(local_file, 'wb') as f:
            f.write(response.content)
        print(f"Downloaded: {local_file}")
    else:
        print(f"Failed to download PDF. Status code: {response.status_code}")
        sys.exit(1)

def open_pdf_in_acrobat(local_file):
    # Open the downloaded PDF in Adobe Acrobat
    subprocess.Popen([r"C:\Program Files\Adobe\Acrobat DC\Acrobat\Acrobat.exe", local_file])


if __name__ == "__main__":

        # Path to your PDF file
    pdf_path = "C:\Program Files\Adobe\Acrobat DC\Acrobat\Javascripts\parameter.pdf"

    # Create a PDF reader object
    reader = PdfReader(pdf_path)

    # Initialize an empty string to store the extracted text
    extracted_text = ""

    # Extract text from the form fields (if any)
    if reader.get_form_text_fields():
        form_fields = reader.get_form_text_fields()
        for field_name, field_value in form_fields.items():
            extracted_text += f"{field_value}"

    # Extract text from all pages
    # for page_num in range(len(reader.pages)):
        page = reader.pages[0]
        extracted_text += page.extract_text()

    # Hardcoded PDF URL
    pdf_url = extracted_text
    print (pdf_url)
    # print(pdf_url)
    
    # Specify the local file path where the PDF will be saved
    local_file = r"C:\Program Files\Adobe\Acrobat DC\Acrobat\Javascripts\downloaded.pdf"  # You can set a more specific path if needed
    
    # Step 1: Download the PDF from the URL
    download_pdf(pdf_url, local_file)
    
    # Step 2: Open the PDF in Adobe Acrobat
    open_pdf_in_acrobat(local_file)
