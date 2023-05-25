import "./Signal.css"
import { Checkbox, Tooltip, Text, TextVariants } from "@patternfly/react-core"

interface SignalObjProps {
   signal: any,
   onCheck: (signal: any) => void,
   isChecked: boolean,
   connections: string[]
}

export const Signal: React.FC<SignalObjProps> = ({ signal, onCheck, isChecked, connections }) => {
   return (
      <Tooltip content={signal.description || "No description"}>
         <div className={"SignalDiv"}>
            <div className={"SignalText"}>
               <Text component={TextVariants.h3} style={{ fontSize: "1rem" }}>{signal.name || "No Name Given"}</Text>
               {connections && connections.length > 0 &&
                  <div style={{ marginTop: "0.5rem" }}>
                     {connections.map((connection: string) => {
                        return <Text component={TextVariants.h6} style={{ fontSize: "0.85rem" }}>{connection}</Text>
                     })}
                  </div>
               }
            </div>
            <Checkbox aria-label="enabled" id="enabled" isChecked={isChecked} onChange={() => onCheck(signal)} />
         </div>
      </Tooltip>
   )
}