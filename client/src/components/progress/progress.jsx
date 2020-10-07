import React, { useState } from "react";
import css from "./progress.module.scss";

export default function Progress() {
  // const [, multiSelected] = useSelectFile();
  const queryProgress = () => {};
  return (
    <div className={css["index"]}>
      <div>总进度</div>
      {/* <div>
        {multiSelected.map((item) => {
          return <div>{item}</div>;
        })}
      </div> */}
    </div>
  );
}
