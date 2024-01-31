/* eslint-disable react/prop-types */
import { useMediaQuery } from '@mui/material';
import React, {
  createContext, useState, useContext, useMemo,
} from 'react';

const DarkModeContext = createContext({ isDark: true });

export const useDarkMode = () => useContext(DarkModeContext);

export function DarkModeProvider({ children }: any) {
  const prefersDarkMode = useMediaQuery('(prefers-color-scheme: dark)');
  const cockpitDark = localStorage.getItem('shell:style');

  const changeDarkMode = (val: string | null, prefersDark: boolean) => {
    if (val === 'auto') {
      return prefersDark;
    }
    if (val === 'light') {
      return false;
    }
    return true;
  };

  const [isDark, setIsDark] = useState(changeDarkMode(cockpitDark, prefersDarkMode));

  // Update dark mode based on local storage changes
  window.addEventListener('storage', (event) => {
    if (event.key === 'shell:style') {
      setIsDark(changeDarkMode(event.newValue, prefersDarkMode));
    }
  });

  const contextValue = useMemo(() => ({ isDark }), [isDark]);

  return (
    <DarkModeContext.Provider value={contextValue}>
      {children}
    </DarkModeContext.Provider>
  );
}
