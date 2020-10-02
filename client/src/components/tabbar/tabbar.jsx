import React, { useState } from "react";
import css from "./tabbar.module.scss";
import { CFG, FUN, BUG } from "../../constants/pages";

import COPYICON from "../../assets/copy.svg";
import ROCKETICON from "../../assets/rocket.svg";
import EXPORTICON from "../../assets/daochu.svg";

function Tabbar() {
  const [curPage, setCurPage] = useState(CFG);
  return (
    <div className={css["index"]}>
      <div className={css["button"]}>
        <img alt="" src={COPYICON} />
        <div style={{ whiteSpace: "pre" }}>{curPage}</div>
      </div>
      <div className={css["button"]}>
        <img alt="" src={ROCKETICON} />
        <div>运行</div>
      </div>
      <div className={css["export"]}>
        <img alt="" src={EXPORTICON} />
        <div>导出结果</div>
      </div>
    </div>
  );
}

export default Tabbar;
