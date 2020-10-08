import React from "react";
import css from "./modal.module.scss";

export default function Modal(props) {
  const {
    title,
    content,
    confirmText,
    cancelText,
    onConfirm,
    top,
    left,
    width,
    height,
    hideCancel,
    dispatchModal,
  } = props;
  return props.visible ? (
    <div className={css["index"]}>
      <div
        className={css["mask"]}
        onClick={() => dispatchModal({ type: "close" })}
      ></div>
      <div className={css["modal"]} style={{ width: width, height: height, top: top, left: left }}>
        <div className={css["title"]}>{title}</div>
        <div className={css["content"]}>{content}</div>
        <div className={css["bottom"]}>
          {hideCancel ? null : <div
            className={`${css["bottom-item"]} ${css["cancel"]}`}
            onClick={() => dispatchModal({ type: "close" })}
          >
            {cancelText}
          </div>}
          <div
            className={`${css["bottom-item"]} ${css["confirm"]}`}
            onClick={onConfirm}
          >
            {confirmText}
          </div>
        </div>
      </div>
    </div>
  ) : null;
}

const noop = (_) => undefined;

Modal.defaultProps = {
  title: "这是标题",
  content: "这是内容啊啊啊",
  onConfirm: noop,
  confirmText: "确定",
  cancelText: "取消",
  hideCancel: false,
  top: '40%',
  left: '40%',
  height: '20%',
  width: '20%'
};
