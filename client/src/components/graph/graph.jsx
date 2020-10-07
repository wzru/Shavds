import React, { useState, useEffect } from "react";
import css from "./graph.module.scss";

import leftIcon from "../../assets/left.svg";
import leftIconGR from "../../assets/left-GR.svg";
import rightIcon from "../../assets/right.svg";
import rightIconGR from "../../assets/right-GR.svg";

export default function Graph(props) {
  const { call, cfg } = props;
  const [curIndex, setCurIndex] = useState(0);
  useEffect(() => {
    setCurIndex(0);
  }, [call, cfg]);
  const onRightClick = () => {
    if (curIndex !== cfg.length - 1) {
      setCurIndex(curIndex + 1);
    }
  };
  const onLeftClick = () => {
    if (curIndex !== 0) {
      setCurIndex(curIndex - 1);
    }
  };
  return (
    <div className={css["index"]}>
      <div className={`${css["pic"]} ${css["call"]}`}>
        <div
          className={css["pic-img"]}
          style={{ backgroundImage: `url(${call[0]})` }}
        ></div>
      </div>
      <div className={`${css["pic"]} ${css["cfg"]}`}>
        <div
          className={css["pic-img"]}
          style={{
            backgroundImage: curIndex === -1 ? "" : `url(${cfg[curIndex]})`,
          }}
        ></div>
        <div className={css["pic-icons"]}>
          <img
            alt=""
            src={curIndex === 0 ? leftIconGR : leftIcon}
            onClick={onLeftClick}
          />
          <img
            alt=""
            src={curIndex === cfg.length - 1 ? rightIconGR : rightIcon}
            onClick={onRightClick}
          />
        </div>
      </div>
    </div>
  );
}
Graph.defaultProps = {
  call: [""],
  cfg: [""],
};
