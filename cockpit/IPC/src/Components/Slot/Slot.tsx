import "./Slot.css"
import { Checkbox, Tooltip } from "@patternfly/react-core"

interface SlotObjProps {
   slot: any,
   onCheck: (slot: any) => void,
   isChecked: boolean
}

export const Slot: React.FC<SlotObjProps> = ({ slot, onCheck, isChecked }) => {
   return (
      <div className={"SlotDiv"}>
         <Checkbox aria-label="enabled" id="enabled" isChecked={isChecked} onChange={() => onCheck(slot)} />
         <Tooltip content={slot.description || "No description"}>
            <h4>{slot.name || "No Name Given"}</h4>
         </Tooltip>
      </div>
   )
}