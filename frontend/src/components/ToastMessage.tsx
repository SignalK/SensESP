import { type JSX } from "preact";
import { useEffect } from "preact/hooks";
import useToast from "./useToast";

interface ToastMessageProps {
  color: string;
  children: React.ReactNode;
  show: boolean;
  autohide?: boolean;
  delay?: number;
  onHide: () => void;
}

export function ToastMessage({
  color,
  children,
  show,
  autohide = true,
  delay = 5000,
  onHide,
}: ToastMessageProps): JSX.Element {
  const { toastRef, showToast, hideToast } = useToast({
    autohide,
    delay,
    onHide,
  });

  useEffect(() => {
    if (show) {
      showToast();
    } else {
      hideToast();
    }
  }, [show]);

  return (
    <>
      <div
        ref={toastRef}
        className={"toast position-fixed border-0 bottom-0 end-0 m-3 " + color}
        style="z-index: 9998"
        role="alert"
        aria-live="assertive"
        aria-atomic="true"
      >
        <div className="d-flex">
          <div className="toast-body">
            <br />
            {children}
          </div>
          <button
            type="button"
            className="btn-close me-2 m-auto"
            data-bs-dismiss="toast"
            aria-label="Close"
            onClick={onHide}
          />
        </div>
      </div>
    </>
  );
}
