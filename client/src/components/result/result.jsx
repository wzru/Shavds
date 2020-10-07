import React from "react";
import css from "./result.module.scss";

export default function Result(props) {
  const { result } = props;
  return (
    <div className={css["index"]}>
      <div className={css["title"]}>比较结果（点击查看详情）</div>
      <div className={css["result"]}>
        {Object.keys(result).map((filenames) => {
          return (
            <div className={css["result-item"]} key={filenames}>
              {filenames}的相似度比较如下
              {result[filenames].map((cmp) => {
                return cmp.similarity !== 0 ? (
                  <div className={css["result-item-item"]} key={cmp.file1 + cmp.file2 + cmp.func1 + cmp.func2}>
                    第<code>{cmp.line1}</code>行的函数<code>{cmp.func1}</code>与第<code>{cmp.line2}</code>行的函数
                    <code>{cmp.func2}</code>
                    的相似度为<code>{cmp.similarity * 100}%</code>
                  </div>
                ) : null;
              })}
            </div>
          );
        })}
      </div>
    </div>
  );
}
