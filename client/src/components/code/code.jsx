import React, { useState } from "react";
import css from "./code.module.scss";

export default function Code() {
  const [code, setCode] = useState("");
  return <div className={css["index"]}>{code ? code : "Hello World!"}</div>;
}
