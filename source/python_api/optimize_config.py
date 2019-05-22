import os
import json
import argparse

from gabac_api import GABAC_TRANSFORM
from gabac_sa import SimulatedAnnealingForGabac

def main(args):
    avail_transform = {
        "NONE"      : GABAC_TRANSFORM.NONE,
        "RLE"       : GABAC_TRANSFORM.RLE,
        "EQUALITY"  : GABAC_TRANSFORM.EQUALITY,
        "MATCH"     : GABAC_TRANSFORM.MATCH
    }

    with open(args.input, 'rb') as f:
        data = f.read()

    if args.algorithm.lower() in ['sa', 'simulated_annealing']:
        for trans_name, trans_id in avail_transform.items():
            sa = SimulatedAnnealingForGabac(
                data, 
                trans_id,
                kmax=200,
                kt=1,
                ena_roundtrip=False,
                verbose=False
            )

            s,E = sa.start()

            with open(os.path.join(args.result, os.path.basename(args.input) + '_' + trans_name + '.json'), 'w') as f:
                json.dump(s, f, indent=4)

            sa.result_as_csv(
                os.path.join(args.result, os.path.basename(args.input) + '_' + trans_name + '.csv')
            )

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Python main for regularized auto-encoder')

    parser.add_argument('-a' , '--algorithm', required=True, type=str, help='Algorithm used for searching the best config')
    #parser.add_argument('--transform', required=True, type=int, help='Gabac Transform')
    parser.add_argument('-i' , '--input', required=True, type=str, help='Path to data')
    parser.add_argument('-r' , '--result', required=True, type=str, help='Path to result')

    args = parser.parse_args()

    main(args)
