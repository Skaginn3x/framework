/* eslint-disable react/function-component-definition */
/* eslint-disable react/jsx-no-useless-fragment */
import React from 'react';
import {
  FormControl, InputLabel, MenuItem, Select, SelectChangeEvent,
} from '@mui/material';
import { useUID } from 'react-uid';
import Qty from 'js-quantities';
import { Units } from './units';

interface UnitsDropdownProps {
  handleUnitChange: (event: SelectChangeEvent<any>, child: React.ReactNode) => void
  initialDimension: string | undefined
  initialUnit: string | undefined
  unit: string | undefined
  disabled: boolean | undefined
}

const UnitsDropdown: React.FC<UnitsDropdownProps> = ({
  handleUnitChange, initialDimension, initialUnit, unit, disabled,
}) => {
  const AllUnits = Units as { [key: string]: { all: string[] } };
  const uid = useUID();

  const [unitsToOffer, setUnitsToOffer] = React.useState<string[] | undefined>([]);

  React.useEffect(() => {
    if (initialDimension && AllUnits[initialDimension]) {
      setUnitsToOffer(AllUnits[initialDimension].all);
    } else if (initialDimension) {
      setUnitsToOffer(Qty.getUnits(initialDimension));
    } else if (initialDimension === undefined) {
      setUnitsToOffer(undefined);
    }
  }, [initialDimension]);

  function getValue() {
    if (!unit || !initialUnit) return undefined;
    const def = Qty.parse(unit ?? initialUnit).units() ?? unit ?? '';
    if (unitsToOffer?.includes(def)) {
      return def;
    }
    const def2 = Qty.parse(unit).numerator[0].slice(1, -1);
    if (unitsToOffer?.includes(def2)) {
      return def2;
    }
    return '';
  }

  return (
    <>
      {initialUnit && unitsToOffer
        ? (
          <FormControl style={{ width: '25%', marginLeft: '0.5rem' }}>
            <InputLabel id="unit-select-label">Unit</InputLabel>
            <Select
              labelId="unit-select-label"
              id="unit-select"
              // eslint-disable-next-line no-underscore-dangle
              value={getValue()}
              onChange={handleUnitChange}
              disabled={disabled}
            >
              {(unitsToOffer && initialDimension && initialUnit)
                ? unitsToOffer.map((mapunit: string) => (
                  <MenuItem key={mapunit + uid} value={mapunit}>
                    {mapunit}
                  </MenuItem>
                )) : null}
            </Select>
          </FormControl>
        )
        : null}
    </>
  );
};

export default UnitsDropdown;
