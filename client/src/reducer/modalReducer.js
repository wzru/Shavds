export default function modalReducer(state, action) {
  switch (action.type) {
    case "open":
      return { ...state, visible: true, ...action.data };
    case "close":
      return { ...state, visible: false };
    default:
      return state;
  }
}
