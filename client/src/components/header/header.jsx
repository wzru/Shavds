import React from "react";
import css from "./header.module.scss";

function Header() {
  return (
    <div className={css["index"]}>
      {"Shavds © \u738b" + "\u8bc1" + "\u5112(U2018" + "\u0031\u0034\u0038\u0036\u0037) 2020软件安全课程设计"}
    </div>
  );
}

export default Header;
