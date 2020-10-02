import React from "react";
import css from "./top.module.scss";

import File from "../file/file";
import Tabbar from "../tabbar/tabbar";
import Code from "../code/code";

export default function Top() {
  return (
    <div className={css["index"]}>
      <Tabbar />
      <File />
      <Code />
    </div>
  );
}
