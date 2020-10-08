import React, { useState, useEffect } from "react";
import css from "./graph.module.scss";

import leftIcon from "../../assets/left.svg";
import leftIconGR from "../../assets/left-GR.svg";
import rightIcon from "../../assets/right.svg";
import rightIconGR from "../../assets/right-GR.svg";
import { URL } from "../../constants/url";

export default function Graph(props) {
  const { call, cfg, dispatchModal } = props;
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
  const onImageClick = () => {
    dispatchModal({
      type: "open",
      data: {
        title: "查看图片",
        top: "10%",
        left: "10%",
        height: "80%",
        width: "80%",
        content: (
          <div
            style={{
              backgroundImage: curIndex === -1 ? "" : `url(${URL}/images/${cfg[curIndex]})`,
              backgroundSize: "contain",
              backgroundPosition: "center center",
              backgroundRepeat: "no-repeat",
              height: "100%",
              width: "100%",
            }}
          ></div>
        ),
        hideCancel: true,
        onConfirm: () => {
          dispatchModal({ type: "close" });
        },
      },
    });
  };
  return (
    <div className={css["index"]}>
      <div className={`${css["pic"]} ${css["call"]}`}>
        {call[0] ? (
          <div className={css["pic-img"]} style={{ backgroundImage: `url(${URL}/images/${call[0]})` }}></div>
        ) : (
          <div>Call Graph</div>
        )}
      </div>
      <div className={`${css["pic"]} ${css["cfg"]}`}>
        {cfg[0] ? (
          <div
            className={css["pic-img"]}
            style={{
              backgroundImage: curIndex === -1 ? "" : `url(${URL}/images/${cfg[curIndex]})`,
            }}
            onClick={onImageClick}
          ></div>
        ) : (
          <div>CFG</div>
        )}
        <div className={css["pic-icons"]}>
          <img alt="" src={curIndex === 0 ? leftIconGR : leftIcon} onClick={onLeftClick} />
          <img alt="" src={curIndex === cfg.length - 1 ? rightIconGR : rightIcon} onClick={onRightClick} />
        </div>
      </div>
    </div>
  );
}
Graph.defaultProps = {
  call: [""],
  cfg: [""],
};
