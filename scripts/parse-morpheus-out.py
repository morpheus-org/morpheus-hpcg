"""
 parse-morphues-out.py
 
 EPCC, The University of Edinburgh
 
 (c) 2022 The University of Edinburgh
 
 Contributing Authors:
 Christodoulos Stylianou (c.stylianou@ed.ac.uk)
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 	http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
"""

import pandas as pd
from io import StringIO
import argparse
import sys

parser = argparse.ArgumentParser(
    description="Script for plotting the sparsity pattern of a matrix from HPCG"
)

parser.add_argument(
    "--filename",
    type=str,
    required=True,
    help="Absolute filename to the file to be processed.",
)

parser.add_argument("--header", action="store_true", default=False)
args = parser.parse_args()

entry = StringIO()
df = (
    pd.read_csv(args.filename, sep="\t")
    .dropna(axis=1)
    .rename(columns=lambda x: x.strip())
    .to_csv(entry, index=False, header=args.header)
)

sys.stdout.write(entry.getvalue())
