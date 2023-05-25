// AlertProvider.tsx
import React, { useReducer } from 'react';
import AlertContext from './AlertContext';

const AlertProvider: React.FC<any> = ({ children }) => {
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

   const addAlert = (title: string, variant: "success" | "default" | "danger" | "warning" | "info" | undefined) => {
      const key = new Date().getTime().toString();
      dispatch({ type: 'add', payload: { title, variant, key } });
      setTimeout(() => dispatch({ type: 'remove', key }), 5000);
   };

   const removeAlert = (key: any) => {
      dispatch({ type: 'remove', key });
   };

   return (
      <AlertContext.Provider value={{ alerts, addAlert, removeAlert }}>
         {children}
      </AlertContext.Provider>
   );
};

export default AlertProvider;
