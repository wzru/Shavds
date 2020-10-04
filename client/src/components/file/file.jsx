import React, { useState } from "react";
import css from "./file.module.scss";

import DELETEICON from "../../assets/delete.svg";
import UPICON from "../../assets/upload.svg";

export default function File() {
  const [list, setList] = useState(["1.cpp", "2.go", "3.js", "4.c"]);
  return (
    <div className={css["index"]}>
      <div className={css["header"]}>
        <div className={css["header-title"]}>文件列表</div>
        <div className={css["header-icons"]}>
          <img className={css["header-icons-item"]} src={DELETEICON} />
          <img className={css["header-icons-item"]} src={UPICON} />
        </div>
      </div>
      <div className={css["list"]}>
        {list.map((item) => {
          return <div className={css["list-item"]}>{item}</div>;
        })}
      </div>
    </div>
  );
}
