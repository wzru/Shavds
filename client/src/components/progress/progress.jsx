import React from "react";
import { BUG } from "../../constants/pages";
import css from "./progress.module.scss";

export default function Progress(props) {
  const { progress, num, curTab } = props;
  return (
    <div className={css["index"]}>
      <div className={css["title"]}>
        总进度{" "}
        {curTab === BUG
          ? `${num}/${num}`
          : `${Object.keys(progress).filter((filenames) => progress[filenames] === 1).length}/${num * (num - 1) * 0.5}`}
      </div>
      <div className={css["progress"]}>
        {Object.keys(progress).map((filenames) => {
          return (
            <div key={filenames} className={css["progress-item"]}>
              <div className={css["progress-item-name"]}>{`${filenames.split("&")[0]}${
                filenames.split("&").length > 1 ? `和${filenames.split("&")[1]}` : ""
              }`}</div>
              <div className={css["progress-item-bar"]}>
                <div
                  className={css["progress-item-bar-content"]}
                  style={{ width: `${progress[filenames] * 100}%` }}
                ></div>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}
