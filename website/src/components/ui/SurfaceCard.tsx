interface SurfaceCardProps {
  children: React.ReactNode;
  className?: string;
  hover?: boolean;
}

export function SurfaceCard({ children, className = '', hover = false }: SurfaceCardProps) {
  return (
    <div
      className={`bg-bg-surface border border-border-default rounded-sm p-6 ${
        hover ? 'hover:border-border-active hover:bg-bg-elevated transition-all duration-200' : ''
      } ${className}`}
    >
      {children}
    </div>
  );
}
