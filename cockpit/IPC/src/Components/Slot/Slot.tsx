import "./Slot.css"
import { Checkbox, Tooltip, Text, TextVariants } from "@patternfly/react-core"

interface SlotObjProps {
   slot: any,
   onCheck: (slot: any) => void,
   isChecked: boolean
}

export const Slot: React.FC<SlotObjProps> = ({ slot, onCheck, isChecked }) => {
   return (
      <Tooltip content={slot.description || "No description"}>
         <div className={"SlotDiv"}>
            <Text component={TextVariants.h3} style={{ fontSize: "1rem" }}>{slot.name || "No Name Given"}</Text>
            <Checkbox aria-label="enabled" id="enabled" isChecked={isChecked} onChange={() => onCheck(slot)} />
         </div>
      </Tooltip>
   )
}