import "./Slot.css"
import { Checkbox, Tooltip } from "@patternfly/react-core"

interface SlotObjProps {
   slot: any,
   onCheck: (slot: any) => void,
   isChecked: boolean
}

export const Slot: React.FC<SlotObjProps> = ({ slot, onCheck, isChecked }) => {
   return (
      <Tooltip content={slot.description || "No description"}>
         <div className={"SlotDiv"}>
            <Checkbox aria-label="enabled" id="enabled" isChecked={isChecked} onChange={() => onCheck(slot)} />
            <h4 style={{ width: "100%", marginRight: "1rem" }}>{slot.name || "No Name Given"}</h4>
         </div>
      </Tooltip>
   )
}