import React, { useReducer, useMemo } from 'react';
import AlertContext from './AlertContext';

export default function AlertProvider({ children }: any) {
  const [alerts, dispatch] = useReducer((state: any, action: any) => {
    switch (action.type) {
      case 'add':
        return [...state, action.payload];
      case 'remove':
        return state.filter((alert: any) => alert.key !== action.key);
      default:
        return state;
    }
  }, []);

  const addAlert = (title: string, variant: 'success' | 'danger' | 'warning' | 'info' | 'custom') => {
    const key = new Date().getTime().toString();
    dispatch({ type: 'add', payload: { title, variant, key } });
    setTimeout(() => dispatch({ type: 'remove', key }), 5000);
  };

  const removeAlert = (key: any) => {
    dispatch({ type: 'remove', key });
  };

  // useMemo will return a memoized version of the object if the alerts, addAlert, removeAlert haven't changed.
  const contextValue = useMemo(() => ({ alerts, addAlert, removeAlert }), [alerts, addAlert, removeAlert]);

  return (
    <AlertContext.Provider value={contextValue}>
      {children}
    </AlertContext.Provider>
  );
}
