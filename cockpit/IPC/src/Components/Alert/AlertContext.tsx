import { createContext, useContext } from "react";
import { AlertVariant } from "@patternfly/react-core";

interface AlertContent {
   title: string;
   variant: AlertVariant;
   key: string;
}

interface AlertContextState {
   alerts: AlertContent[];
   addAlert: (title: string, variant: AlertVariant) => void;
   removeAlert: (key: string) => void;
}

// Provide initial values for the context.
const AlertContext = createContext<AlertContextState>({
   alerts: [],
   addAlert: () => { },
   removeAlert: () => { },
});

export const useAlertContext = () => useContext(AlertContext);

export default AlertContext;
