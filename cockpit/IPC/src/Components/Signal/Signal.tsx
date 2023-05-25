import "./Signal.css"
import { Checkbox, Tooltip } from "@patternfly/react-core"

interface SignalObjProps {
   signal: any,
   onCheck: (signal: any) => void,
   isChecked: boolean
}

export const Signal: React.FC<SignalObjProps> = ({ signal, onCheck, isChecked }) => {
   return (
      <div className={"SignalDiv"}>
         <Tooltip content={signal.description || "No description"}>
            <h4 style={{ width: "100%", marginRight: "1rem" }}>{signal.name || "No Name Given"}</h4>
         </Tooltip>
         <Checkbox aria-label="enabled" id="enabled" isChecked={isChecked} onChange={() => onCheck(signal)} />
      </div>
   )
}