export default interface ErrorType {
   problem: string | null | undefined,
   name: string,
   message: string,
}

export default interface SingalType {
   name: string,
   description?: string,
   type: "unknown" | "bool" | "int64_t" | "uint64_t" | "double" | "string" | "json",
   created_by: string,
   created_at: string,
   last_registered: string,
}

export default interface SlotType {
   name: string,
   description?: string,
   type: "unknown" | "bool" | "int64_t" | "uint64_t" | "double" | "string" | "json",
   created_by: string,
   created_at: string,
   last_registered: string,
   last_modified: string,
}